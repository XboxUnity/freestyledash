#include "stdafx.h"
#include "Drive.h"

HRESULT Drive::Mount()
{
	// Get our serial
	LoadSerial();

	// Get our serial as a string
	CHAR serialStr[41]; serialStr[40] = 0; UINT outLen = 0x40;
	GetBytesString(m_Serial, 0x14, serialStr, &outLen);
	m_SerialStr = (string)serialStr;
	if (m_DriveType == DEVICE_USBMEMORY_UNIT0 || 
		m_DriveType == DEVICE_USBMEMORY_UNIT1 ||
		m_DriveType == DEVICE_USBMEMORY_UNIT2 )
		m_SerialStr = m_SerialStr + "_USBMU";
	if (m_DriveType == DEVICE_USBMEMORY_Cache0 ||
		m_DriveType == DEVICE_USBMEMORY_Cache1 ||
		m_DriveType == DEVICE_USBMEMORY_Cache2 )
		m_SerialStr = m_SerialStr + "_USBMUC";

	//DebugMsg("Mounting %s",m_MountPoint.c_str());
	char MountConv[260];
	sprintf_s( MountConv, "\\??\\%s", m_MountPoint.c_str() );
	char SysPath[260];
	sprintf_s( SysPath,"%s", m_SystemPath.c_str() );

	STRING sSysPath = { (USHORT)strlen( SysPath ), (USHORT)strlen( SysPath ) + 1, SysPath };
	STRING sMountConv = { (USHORT)strlen( MountConv ), (USHORT)strlen( MountConv ) + 1, MountConv };
	
	//don't try to mount 'Game:' as it's already there
	if (stricmp(m_MountPoint.c_str(), "Game:") != 0)
	{
		int res = ObCreateSymbolicLink( &sMountConv, &sSysPath );
	
		if (res != 0)
		{
			//DebugMsg("Mount failed : %d",res);
			return res;
		}
	}

	HRESULT ret = IsMounted();
	if(ret == 1)
	{
		GetDiskFreeSpaceEx(getRootPath().c_str(),&m_FreeBytesAvailable,&m_TotalNumberOfBytes,&m_TotalNumberOfFreeBytes);
	}
	else
	{
		m_FreeBytesAvailable.QuadPart = 0;
		m_TotalNumberOfBytes.QuadPart =0;
		m_TotalNumberOfFreeBytes.QuadPart=0;

	}
	return ret;
}

string Drive::GetCleanDriveName()
{
	return str_replaceallA(m_MountPoint,":","");
}

HRESULT Drive::Unmount( )
{
	char MountConv[16];
	sprintf_s( MountConv,"\\??\\%s", m_MountPoint.c_str() );

	STRING sMountConv = { (USHORT)strlen(MountConv), (USHORT)strlen(MountConv) + 1, MountConv };
	return ( HRESULT )  ObDeleteSymbolicLink( &sMountConv );
}

int Drive::IsMounted()
{
	//MATTIE: changed mounted test to also report empty drives as 
	//mounted, instead of searching for a file on a drive. 
	int Result = 0; 
	if (FileExistsA(m_MountPoint + "\\")) 
	{ 
		Result = 1;
	} 
	return Result;
}

BOOL Drive::LoadSerial(){

	// Clear our serial
	ZeroMemory(m_Serial, 0x14);

	if(m_DriveType == DEVICE_CDROM0) {
		memcpy(m_Serial, "DVDROM", 6);
		return TRUE;
	}

	// Make sure we can get a serial for this
	if( m_DriveType != DEVICE_NAND_FLASH &&
		m_DriveType != DEVICE_MEMORY_UNIT0 && m_DriveType != DEVICE_MEMORY_UNIT1 &&
		m_DriveType != DEVICE_MEMORY_ONBOARD && m_DriveType != DEVICE_USB0 &&
		m_DriveType != DEVICE_USB1 && m_DriveType != DEVICE_USB2 &&
		m_DriveType != DEVICE_HARDISK0_PART1 && m_DriveType != DEVICE_HARDISK0_SYSPART &&
		m_DriveType != DEVICE_USBMEMORY_UNIT0 && m_DriveType != DEVICE_USBMEMORY_UNIT1 && m_DriveType != DEVICE_USBMEMORY_UNIT2 &&
		m_DriveType != DEVICE_USBMEMORY_Cache0 && m_DriveType != DEVICE_USBMEMORY_Cache1 && m_DriveType != DEVICE_USBMEMORY_Cache2)
		return TRUE;

	// Check if it is our flash
	if(m_DriveType == DEVICE_NAND_FLASH) {
		BYTE xboxSerial[0x0C]; DWORD serialLength = 0x0C;
		if(XeKeysGetKey(0x14, xboxSerial, &serialLength) != 0) return FALSE;
		XeCryptSha(xboxSerial, 0x0C, NULL, 0, NULL, 0, m_Serial, 0x14);
		return TRUE;
	}

	// Lets open our device
	HANDLE fHandle = NULL;	STRING devicePath; IO_STATUS_BLOCK ioStatus;
	RtlInitAnsiString(&devicePath, m_SystemPath.c_str());
	OBJECT_ATTRIBUTES objAtt = {NULL, &devicePath, OBJ_CASE_INSENSITIVE};
	if(NtOpenFile(&fHandle, -0x7FF0, &objAtt, &ioStatus, 3, 0x20) != STATUS_SUCCESS)
		return FALSE;

	// Figure out our control code
	ULONG ioCtrlCode = 0;
	switch(m_DriveType) {
		case DEVICE_MEMORY_UNIT0:
		case DEVICE_MEMORY_UNIT1:
		case DEVICE_MEMORY_ONBOARD:
			ioCtrlCode = 0x3A0004;
			break;
		case DEVICE_USB0:
		case DEVICE_USB1:
		case DEVICE_USB2:
		case DEVICE_USBMEMORY_UNIT0:
		case DEVICE_USBMEMORY_UNIT1:
		case DEVICE_USBMEMORY_UNIT2:
		case DEVICE_USBMEMORY_Cache0:
		case DEVICE_USBMEMORY_Cache1:
		case DEVICE_USBMEMORY_Cache2:
			ioCtrlCode = 0x2D0010;
			break;
		case DEVICE_HARDISK0_PART1:
		case DEVICE_HARDISK0_SYSPART:
			ioCtrlCode = 0x4D028;
			break;
		default:
			return TRUE;
	}

	// If its not our HDD lets just do a simple version
	if(m_DriveType != DEVICE_HARDISK0_PART1 && m_DriveType != DEVICE_HARDISK0_SYSPART) {
		
		// Get our serial 
		NTSTATUS status = NtDeviceIoControlFile(fHandle, NULL, NULL, NULL, 
			&ioStatus, ioCtrlCode, NULL, 0, m_Serial, 0x14);
		
		// Close our device and return status
		NtClose(fHandle);
		return status == STATUS_SUCCESS;
	}

	// Its a hdd so lets setup a request for our info
	BYTE ideIdentifyData[0x200];
	REQUEST_PACKET rp; ZeroMemory(&rp, 0x10);
	rp.code = 0xEC;	rp.size = 0x200;
	rp.pointer = ideIdentifyData;	

	// Now lets make the request
	NTSTATUS status = NtDeviceIoControlFile(fHandle, NULL, NULL, NULL, 
		&ioStatus, ioCtrlCode, &rp, 0x10, &rp, 0x10);
	
	// Close our device
	NtClose(fHandle);

	// If its a success lets copy in our serial
	if(status == STATUS_SUCCESS) {

		// Copy our serial from the HDD
		memcpy(m_Serial, ideIdentifyData + 0x14, 0x14);

		// If its the system ext lets change it so it wont conflict with partition1
		if(m_DriveType == DEVICE_HARDISK0_SYSPART)
			XeCryptSha(m_Serial, 0x14, NULL, 0, NULL, 0, m_Serial, 0x14);
		return TRUE;
	}

	// We shouldent get here :(
	return FALSE;
};