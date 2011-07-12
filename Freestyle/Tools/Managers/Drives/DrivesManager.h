#pragma once

#include "../../Generic/Tools.h"
#include "../../Generic/XboxTools.h"
#include "Drive.h"
/*
static const DriveLookup m_DrivesIdLookup[] = {
"Flash:", DEVICE_NAND_FLASH, "\\Device\\Flash",
"Memunit0:", DEVICE_MEMORY_UNIT0, "\\Device\\Mu0",
"Memunit1:",  DEVICE_MEMORY_UNIT1,  "\\Device\\Mu1",
"OnBoardMU:", DEVICE_MEMORY_ONBOARD, "\\Device\\BuiltInMuSfc",
"Dvd:", DEVICE_CDROM0, "\\Device\\Cdrom0",
"Hdd1:", DEVICE_HARDISK0_PART1,"\\Device\\Harddisk0\\Partition1",
"HddX:", DEVICE_HARDISK0_SYSPART, "\\Device\\Harddisk0\\SystemPartition",
"Usb0:", DEVICE_USB0,"\\Device\\Mass0",
"Usb1:",  DEVICE_USB1,  "\\Device\\Mass1",
"Usb2:", DEVICE_USB2,	"\\Device\\Mass2",
};

*/
class DrivesManager
{
protected:
	DrivesManager();
	// Private constructor
	~DrivesManager() {}
	DrivesManager(const DrivesManager&);                 // Prevent copy-construction
	DrivesManager& operator=(const DrivesManager&);      // Prevent assignment

	std::vector<Drive* const> m_Drives;
	int m_Mounted[20];
	string m_ExecutableFilePath;

	void Init();
	void MountAll();

public :
	void getMountedDrives(std::vector<Drive* const> *retVal);
	Drive* getDriveBySystemPath(string SystemPath);
	Drive* getDriveByMountPoint(string MountPoint);
	Drive* getDriveBySerialNumberStr( string szSerialNumber );
	
	string getRootPathBySerialNumberStr( string szSerialNumber );
	string getExecutablePathFromSystemPath(string ExecutableSystemPath);
	string getExecutablePath();
	Drive* getDriveByExecutableSystemPath( string ExecutableSystemPath);
	Drive* getDriveByExecutablePath( string ExecutablePath);

	static DrivesManager& getInstance()
	{
		static DrivesManager singleton;
		return singleton;
	}

};