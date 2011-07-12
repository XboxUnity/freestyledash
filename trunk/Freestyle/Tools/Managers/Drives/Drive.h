#pragma once
#include "../../Generic/Tools.h"
#include "../../Generic/XboxTools.h"
#include "../../Debug/Debug.h"

typedef enum 
{
    DEVICE_NAND_FLASH, 
	DEVICE_MEMORY_UNIT0, 
	DEVICE_MEMORY_UNIT1,  
	DEVICE_MEMORY_ONBOARD, 
    DEVICE_CDROM0, 
	DEVICE_HARDISK0_PART1, 
	DEVICE_HARDISK0_PART0,
	DEVICE_HARDISK0_SYSPART, 
	DEVICE_HARDISK0_SYSEXT,
	DEVICE_USB0,
    DEVICE_USB1,  
	DEVICE_USB2, 
	DEVICE_HDDVD_PLAYER,         
    DEVICE_HDDVD_STORGE,
    DEVICE_TRANSFER_CABLE,
    DEVICE_TRANSFER_CABLE_SYSPART,
	DEVICE_TEST,
	DEVICE_USBMEMORY_UNIT0,
	DEVICE_USBMEMORY_UNIT1,
	DEVICE_USBMEMORY_UNIT2,
	DEVICE_USBMEMORY_Cache0,
	DEVICE_USBMEMORY_Cache1,
	DEVICE_USBMEMORY_Cache2
} DriveType;


class Drive
{
private:
	typedef struct _REQUEST_PACKET{
		DWORD unknown;
		WORD  unknown2;
		BYTE  code;
		BYTE  unknown3;
		DWORD size;
		PVOID pointer;
	} REQUEST_PACKET;

	BOOL LoadSerial();
protected :
	string m_MountPoint;
	string m_SystemPath;
	DriveType m_DriveType;
	
	ULARGE_INTEGER m_FreeBytesAvailable;
	ULARGE_INTEGER m_TotalNumberOfBytes;
	ULARGE_INTEGER m_TotalNumberOfFreeBytes;
	BYTE m_Serial[0x14];
	string m_SerialStr;

public :
	Drive(string MountPoint,string SystemPath,DriveType DriveType)
	{
		m_MountPoint = MountPoint;
		m_SystemPath = SystemPath;
		m_DriveType = DriveType;
	}

	string getMountPoint() { return m_MountPoint; }
	string getSystemPath() { return m_SystemPath; }
	string getRootPath() { return m_MountPoint + "\\"; }
	ULONGLONG getTotalNumberOfBytes() {	return m_TotalNumberOfBytes.QuadPart; }
	ULONGLONG getFreeNumberOfBytes() { return m_TotalNumberOfFreeBytes.QuadPart; }
	ULONGLONG getUsedNumberOfBytes() { return m_TotalNumberOfBytes.QuadPart-m_TotalNumberOfFreeBytes.QuadPart; }
	PBYTE getSerial() { return m_Serial; }
	string getSerialStr() { return m_SerialStr; }
	DriveType GetDriveType() { return m_DriveType; }

    HRESULT Mount();
  	string GetCleanDriveName();
    HRESULT Unmount();
    int IsMounted();
	
};