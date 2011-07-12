#include "stdafx.h"
#include "../../Generic/xboxtools.h"
#include "DrivesManager.h"
#include "../../SQLite/FSDSql.h"

DrivesManager::DrivesManager()
{
	Init();
}


void DrivesManager::Init()
{
	m_Drives.push_back(new Drive("Flash:","\\Device\\Flash",DEVICE_NAND_FLASH));
	m_Drives.push_back(new Drive("Memunit0:", "\\Device\\Mu0",DEVICE_MEMORY_UNIT0));
	m_Drives.push_back(new Drive("Memunit1:", "\\Device\\Mu1",DEVICE_MEMORY_UNIT1));
	m_Drives.push_back(new Drive("OnBoardMU:", "\\Device\\BuiltInMuSfc",DEVICE_MEMORY_ONBOARD));
	m_Drives.push_back(new Drive("Dvd:", "\\Device\\Cdrom0",DEVICE_CDROM0));
	m_Drives.push_back(new Drive("Hdd1:", "\\Device\\Harddisk0\\Partition1",DEVICE_HARDISK0_PART1));
	m_Drives.push_back(new Drive("Hdd0:", "\\Device\\Harddisk0\\Partition0", DEVICE_HARDISK0_PART0));
	m_Drives.push_back(new Drive("HddX:", "\\Device\\Harddisk0\\SystemPartition",DEVICE_HARDISK0_SYSPART));
	m_Drives.push_back(new Drive("SysExt:", "\\Device\\Harddisk0\\SystemExtPartition", DEVICE_HARDISK0_SYSEXT));
	m_Drives.push_back(new Drive("Usb0:", "\\Device\\Mass0",DEVICE_USB0));
	m_Drives.push_back(new Drive("Usb1:", "\\Device\\Mass1",DEVICE_USB1));
	m_Drives.push_back(new Drive("Usb2:", "\\Device\\Mass2",DEVICE_USB2));
	m_Drives.push_back(new Drive("HdDvdPlayer:", "\\Device\\HdDvdPlayer",DEVICE_HDDVD_PLAYER));
	m_Drives.push_back(new Drive("HdDvdStorage:", "\\Device\\HdDvdStorage",DEVICE_HDDVD_STORGE));
	m_Drives.push_back(new Drive("Transfercable:", "\\Device\\Transfercable",DEVICE_TRANSFER_CABLE)); //only with 2.0.9199 Kernel
	m_Drives.push_back(new Drive("TransfercableXbox1:", "\\Device\\Transfercable\\Compatibility\\Xbox1",DEVICE_TRANSFER_CABLE_SYSPART)); //only with 2.0.9199 Kernel  
	m_Drives.push_back(new Drive("USBMU0:", "\\Device\\Mass0PartitionFile\\Storage",DEVICE_USBMEMORY_UNIT0));
	m_Drives.push_back(new Drive("USBMU1:", "\\Device\\Mass1PartitionFile\\Storage",DEVICE_USBMEMORY_UNIT1));
	m_Drives.push_back(new Drive("USBMU2:", "\\Device\\Mass2PartitionFile\\Storage",DEVICE_USBMEMORY_UNIT2));
	m_Drives.push_back(new Drive("USBMUCache0:", "\\Device\\Mass0PartitionFile\\StorageSystem",DEVICE_USBMEMORY_Cache0));
	m_Drives.push_back(new Drive("USBMUCache1:", "\\Device\\Mass1PartitionFile\\StorageSystem",DEVICE_USBMEMORY_Cache1));
	m_Drives.push_back(new Drive("USBMUCache2:", "\\Device\\Mass2PartitionFile\\StorageSystem",DEVICE_USBMEMORY_Cache2));

	// Set variable to represent current xexpath
	string LoadedImageName(GetLoadedImageName());

	m_ExecutableFilePath = getExecutablePathFromSystemPath(LoadedImageName);
	MountAll();

	//add Game: to the drives as well (has to be done after the rest is mounted)
	/*
	Drive* ExecutableDrive = getDriveByExecutablePath(m_ExecutableFilePath);
	string ExecutableFolderSystemPath = LoadedImageName.substr(0, LoadedImageName.find_last_of("\\"));
	Drive* dr = new Drive("Game:", ExecutableFolderSystemPath, ExecutableDrive->GetDriveType());
	m_Drives.push_back(dr);
	dr->Mount();
	*/

	//Tell the database about all of the mounted devices, 
	for(unsigned int x=0;x<m_Drives.size();x++)
	{
		if(m_Drives[x]->IsMounted())
		{
			FSDSql::getInstance().addMountedDevice(m_Drives[x]->getMountPoint(), m_Drives[x]->getSerialStr());
		}
	}
}

Drive * DrivesManager::getDriveBySerialNumberStr( string szSerialNumber )
{
	Drive * retDrive = NULL;
	vector<Drive* const> mountedDriveList;
	getMountedDrives(&mountedDriveList);

	vector<Drive* const>::iterator itr;
	for( itr = mountedDriveList.begin(); itr != mountedDriveList.end(); ++itr )
	{
		Drive * tempDrive = (*itr);
		string szTempSerial = tempDrive->getSerialStr();
		if(strcmp(szTempSerial.c_str(), szSerialNumber.c_str()) == 0) {
			retDrive = tempDrive;
			break;
		}
	}

	return retDrive;
}

string DrivesManager::getRootPathBySerialNumberStr( string szSerialNumber )
{
	string szRetVal = "";

	Drive * targetDrive = getDriveBySerialNumberStr(szSerialNumber);
	if( targetDrive != NULL ) 
	{
		szRetVal = targetDrive->getRootPath();
		szRetVal = szRetVal.substr(0, szRetVal.length() - 1);
	}

	// This will return either "hdd1:\\, usb0:\\, usb1:\\, etc
	return szRetVal;
}

void DrivesManager::MountAll()
{
	for(unsigned int x=0;x<m_Drives.size();x++)
	{
		Drive* dr = m_Drives[x];
		dr->Mount();
	}
}

void DrivesManager::getMountedDrives(std::vector<Drive* const> *retVal)
{
	bool bShowAdvancedDrives = SETTINGS::getInstance().getFMAdvancedMode() == 0 ? false : true;

	for(unsigned int x=0;x<m_Drives.size();x++)
	{
		if(m_Drives[x]->IsMounted())
		{
			Drive * drv = m_Drives[x];
			if( drv->GetDriveType() == DEVICE_NAND_FLASH && !bShowAdvancedDrives )				//flash
				continue;
			if( drv->GetDriveType() == DEVICE_HARDISK0_SYSEXT && !bShowAdvancedDrives )			//sysext
				continue;
			if( drv->GetDriveType() == DEVICE_HARDISK0_SYSPART && !bShowAdvancedDrives )		//hddx
				continue;

			retVal->push_back(m_Drives[x]);

		}
		//Drive* dr = m_Drives[x];
		//dr->Mount();
	}

}

Drive* DrivesManager::getDriveByExecutableSystemPath( string ExecutableSystemPath)
{
	ExecutableSystemPath = make_lowercaseA(ExecutableSystemPath);
	Drive * ret = NULL;
	size_t found;
	for(unsigned int x = 0; x < m_Drives.size(); x++)
	{
		found = ExecutableSystemPath.find(make_lowercaseA(m_Drives[x]->getSystemPath()));
		if(found != ExecutableSystemPath.npos)
		{
			ret = m_Drives[x];
			break;
		}
	}
	
	return ret;
}

string DrivesManager::getExecutablePath()
{
	return m_ExecutableFilePath;
}

string DrivesManager::getExecutablePathFromSystemPath(string ExecutableSystemPath)
{
	string ret = "";
	Drive * drive = getDriveByExecutableSystemPath(ExecutableSystemPath);
	if(drive == NULL)
		return "";

	ExecutableSystemPath.replace(0, drive->getSystemPath().length(), drive->getRootPath());
	ret = str_replaceallA(ExecutableSystemPath, "\\\\", "\\");

	return make_lowercaseA(ret);
}

Drive* DrivesManager::getDriveBySystemPath(string SystemPath)
{
	Drive * ret = NULL;

	for(unsigned int x = 0; x < m_Drives.size(); x++)
	{
		if(strcmp(m_Drives[x]->getSystemPath().c_str(), SystemPath.c_str()) == 0)
		{
			ret = m_Drives[x];
			break;
		}
	}
	
	return ret;
}

Drive* DrivesManager::getDriveByMountPoint(string MountPoint)
{
	Drive *ret = NULL;
	for(unsigned int x=0;x<m_Drives.size();x++)
	{
		if(stricmp(m_Drives[x]->getMountPoint().c_str(),MountPoint.c_str())==0)
		{
			ret = m_Drives[x];
			break;
		}
		//Drive* dr = m_Drives[x];
		//dr->Mount();
	}
	return ret;
}

Drive* DrivesManager::getDriveByExecutablePath( string ExecutablePath)
{
	ExecutablePath = make_lowercaseA(ExecutablePath);
	Drive * ret = NULL;
	size_t found;
	for(unsigned int x = 0; x < m_Drives.size(); x++)
	{
		found = ExecutablePath.find(make_lowercaseA(m_Drives[x]->getMountPoint()));
		if(found != ExecutablePath.npos)
		{
			ret = m_Drives[x];
			break;
		}
	}
	
	return ret;
}