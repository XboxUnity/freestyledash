#include "stdafx.h"
#include "ScnSystemInfo.h"

#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SMC/smc.h"
#include "../../Tools/Managers/Drives/DrivesManager.h"
#include "../../Tools/Managers/Drives/Drive.h"
#include "../../Tools/Managers/VariablesCache/VariablesCache.h"
#include "../../Tools/Monitors/Status/StatusMonitor.h"

#define TM_CLOCK 123456
HRESULT CScnSysInfo::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	fastMode = false;
	showDebugMsg = true; // set to true to show debug messages

	if(showDebugMsg)
		DebugMsg("ScnSysInfo","CScnSysInfo::OnInit v2");

	LoadSettings("ScnSysInfo", *this);

	InitializeChildren();

	ParseSettings();

	SetClock();
	SetTemps();
	SetIp();
	UpdateStorageInfo();
	
	UpdateSMCStructure(true);
	SetInfoText();

	SetTimer(TM_CLOCK,1000);
	


    return S_OK;
}

HRESULT CScnSysInfo::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
    switch( pTimer->nId )
    {
    case TM_CLOCK:
		SetClock();
		SetTemps();
		UpdateSMCStructure(false);
		SetInfoText();
		SetCurrentStatus( );
		bHandled = TRUE;

		SetIp();
		break;
	
	}


    return S_OK;
}
void CScnSysInfo::SetCurrentStatus()
{
	if(hasCurrentStatus)
	{
		
		string status = StatusMonitor::getInstance().GetStatus();
		
		

		if(status == "")
		{
			status = "Idle";
			if(fastMode)
			{
				fastMode = false;
				SetTimer(TM_CLOCK,1000);
			}
		}
		else
		{
			if(!fastMode)
			{
				fastMode = true;
				SetTimer(TM_CLOCK,200);
			}
		}
		m_CurrentStatus.SetText(strtowstr(status).c_str());
	}
}

void CScnSysInfo::ParseSettings( void )
{
	string gsResult;

	// Ex. Set True to remove "CPU:" from tempature.
	gsResult = GetSetting( "OVERRIDETEMPLABEL", "FALSE");
	if(strcmp(gsResult.c_str(),"TRUE")==0)
		bOverrideTempLabel = true;
	else
		bOverrideTempLabel = false;

	// Ex. Set True to remove "F" (tempature units).
	gsResult = GetSetting( "OVERRIDETEMPUNIT", "FALSE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bOverrideTempUnit = true;
	else
		bOverrideTempUnit = false;

	// Ex. Set False to Keep Icons when Temp Display is turned off in Settings
	gsResult = GetSetting( "DISABLETEMPICONS", "TRUE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bDisableTempIcons = true;
	else
		bDisableTempIcons = false;

	// Ex. Set False to Keep Icons when IP Display is turned off in Settings
	gsResult = GetSetting( "DISABLEIPICON", "TRUE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bDisableIPAddrIcons = true;
	else
		bDisableIPAddrIcons = false;

	// Ex. Set True to remove "HDD1:" from storage stats.
	gsResult = GetSetting( "OVERRIDESTORAGELABEL", "FALSE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bOverrideStorageLabel = true;
	else
		bOverrideStorageLabel = false;

	gsResult = GetSetting( "OVERRIDESTORAGEUNIT", "FALSE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bOverrideStorageUnits = true;
	else
		bOverrideStorageUnits = false;

	// Ex. Set True to remove "HDD1:" from storage stats.
	gsResult = GetSetting( "UNMOUNTEDTEXT", "NULL");
	if(strcmp(gsResult.c_str(), "NULL")==0)
	{
		bUseUnmountedText = false;
		sNotMountedText = "";
	}else
	{
		bUseUnmountedText = true;
		sNotMountedText = gsResult;
	}

	// Ex. Set False to Keep Icons when Storage is turned off in Settings
	gsResult = GetSetting( "NOICONUNMOUNTED", "FALSE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bDisableStorageIcons = true;
	else
		bDisableStorageIcons = false;

	gsResult = GetSetting( "NOGRAPHUNMOUNTED", "FALSE");
	if(strcmp(gsResult.c_str(), "TRUE")==0)
		bShowGraphUnmounted = false;
	else
		bShowGraphUnmounted = true;

	gsResult = GetSetting( "PERCENTTODISPLAY", "USED");
	if(strcmp(gsResult.c_str(), "USED")==0)
		nPercentageType = SO_USEDPERCENT;
	else
		nPercentageType = SO_FREEPERCENT;

	gsResult = GetSetting( "STORAGETODISPLAY", "FREE");
	if(strcmp(gsResult.c_str(), "FREE")==0)
		nDisplayType = SO_FREE;
	else
		nDisplayType = SO_USED;

	gsResult = GetSetting( "STORAGEUNITLOCATION", "SECOND");
	if(strcmp(gsResult.c_str(), "SECOND")==0)
		nUnitLocation = SO_UNITSECOND;
	else if(strcmp(gsResult.c_str(), "FIRST")==0)
		nUnitLocation = SO_UNITFIRST;
	else if(strcmp(gsResult.c_str(), "BOTH")==0)
		nUnitLocation = SO_UNITBOTH;
	else
		nUnitLocation = SO_UNITNONE;
}

void CScnSysInfo::SetClock( void )
{
	// Time, Date, and DateTime Icons not Yet Used
	if(hasDate[0] || hasTime[0] || hasDateTime[0])
	{
		SYSTEMTIME LocalSysTime;
		  
		GetLocalTime( &LocalSysTime );
		string time = GetTime(SETTINGS::getInstance().getTimeFormat(), LocalSysTime);
		string date = GetDate(SETTINGS::getInstance().getDateFormat(), LocalSysTime);

		if(hasDate[0])
			m_date.SetText(strtowstr(date).c_str());

		if(hasTime[0])
			m_time.SetText(strtowstr(time).c_str());

		if(hasDateTime[0])
			m_datetime.SetText(strtowstr((date + " " + time)).c_str());
	}
}

float CScnSysInfo::UpdateTempGraphs( int deviceType )
{

	float fTemp = 0.0f;


	switch (deviceType)
	{
	case CPU:
		fTemp = StatusMonitor::getInstance().GetCPUTemp();
		if(hasProgressBar[CPU])
		{
			m_CPUProgressBar.SetValue(StatusMonitor::getInstance().GetCPUTPercent());
		}
		break;
	case GPU:
		fTemp = StatusMonitor::getInstance().GetGPUTemp();
		if(hasProgressBar[GPU])
		{
			m_GPUProgressBar.SetValue(StatusMonitor::getInstance().GetGPUTPercent());
		}
		break;
	case MEM:
		fTemp = StatusMonitor::getInstance().GetMEMTemp();
		if(hasProgressBar[MEM])
		{
			m_RAMProgressBar.SetValue(StatusMonitor::getInstance().GetMEMTPercent());
		}
		break;
	case BRD:
		fTemp =StatusMonitor::getInstance().GetCASETemp();
		if(hasProgressBar[BRD])
		{
			m_MBProgressBar.SetValue(StatusMonitor::getInstance().GetCASETPercent());
		}
		break;
	}

	return fTemp;
}

void CScnSysInfo::SetTemps( void ) 
{
	bool useCelsius;
	float fTemp;

	if(SETTINGS::getInstance().getCels() == 1)
		useCelsius = true;
	else
		useCelsius = false;

	if(hasCPUTemp[0] || hasGPUTemp[0] || hasMBTemp[0] || hasRAMTemp[0])
	{
		string CPUString;
		string GPUString;
		string EDRAMString;
		string MBString;

		if(SETTINGS::getInstance().getTempStatus() == 1)
		{
			if(bOverrideTempLabel)
			{
				if(bOverrideTempUnit)
				{			
					if(hasCPUTemp[0])
					{
						fTemp = UpdateTempGraphs(CPU);
						CPUString = sprintfa("%0.1f",fTemp);
					}
					if(hasGPUTemp[0])
					{
						fTemp = UpdateTempGraphs(GPU);
						GPUString = sprintfa("%0.1f", fTemp);
					}
					if(hasRAMTemp[0])
					{
						fTemp = UpdateTempGraphs(MEM);
						EDRAMString = sprintfa("%0.1f", fTemp);
					}
					if(hasMBTemp[0])
					{
						fTemp = UpdateTempGraphs(BRD);
						MBString = sprintfa("%0.1f", fTemp);
					}
				}else{
					if(hasCPUTemp[0])
					{
						fTemp = UpdateTempGraphs(CPU);
						CPUString = sprintfa("%0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
					if(hasGPUTemp[0])
					{
						fTemp = UpdateTempGraphs(GPU);
						GPUString = sprintfa("%0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
					if(hasRAMTemp[0])
					{
						fTemp = UpdateTempGraphs(MEM);
						EDRAMString = sprintfa("%0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
					if(hasMBTemp[0])
					{
						fTemp = UpdateTempGraphs(BRD);
						MBString = sprintfa("%0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
				}
			}else{
				if(bOverrideTempUnit)
				{			
					if(hasCPUTemp[0])
					{
						fTemp = UpdateTempGraphs(CPU);
						CPUString = sprintfa("CPU: %0.1f",fTemp);
					}
					if(hasGPUTemp[0])
					{
						fTemp = UpdateTempGraphs(GPU);
						GPUString = sprintfa("GPU: %0.1f", fTemp);
					}
					if(hasRAMTemp[0])
					{
						fTemp = UpdateTempGraphs(MEM);
						EDRAMString = sprintfa("EDRAM: %0.1f", fTemp);
					}
					if(hasMBTemp[0])
					{
						fTemp = UpdateTempGraphs(BRD);
						MBString = sprintfa("MB: %0.1f", fTemp);
					}
				}else{
					if(hasCPUTemp[0])
					{
						fTemp = UpdateTempGraphs(CPU);
						CPUString = sprintfa("CPU: %0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
					if(hasGPUTemp[0])
					{
						fTemp = UpdateTempGraphs(GPU);
						GPUString = sprintfa("GPU: %0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
					if(hasRAMTemp[0])
					{
						fTemp = UpdateTempGraphs(MEM);
						EDRAMString = sprintfa("EDRAM: %0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
					if(hasMBTemp[0])
					{
						fTemp = UpdateTempGraphs(BRD);
						MBString = sprintfa("MB: %0.1f%s", fTemp, (useCelsius==true) ? " C" : " F");
					}
				}
			}
			if(hasCPUTemp[0])
				m_CPUTemp.SetText(strtowstr(CPUString).c_str());
			if(hasGPUTemp[0])
				m_GPUTemp.SetText(strtowstr(GPUString).c_str());
			if(hasRAMTemp[0])
				m_RAMTemp.SetText(strtowstr(EDRAMString).c_str());
			if(hasMBTemp[0])
				m_MBTemp.SetText(strtowstr(MBString).c_str());
			
			if(hasCPUTemp[1])
				m_CPUTempIcon.SetShow(true);
			if(hasGPUTemp[1])
				m_GPUTempIcon.SetShow(true);
			if(hasRAMTemp[1])
				m_RAMTempIcon.SetShow(true);
			if(hasMBTemp[1])
				m_MBTempIcon.SetShow(true);

		}else{
			
			if(hasCPUTemp[0])
				m_CPUTemp.SetText(NULL);
			if(hasGPUTemp[0])
				m_GPUTemp.SetText(NULL);
			if(hasRAMTemp[0])
				m_RAMTemp.SetText(NULL);
			if(hasMBTemp[0])
				m_MBTemp.SetText(NULL);

			if(bDisableTempIcons)
			{			

				if(hasCPUTemp[1])
					m_CPUTempIcon.SetShow(false);
				if(hasGPUTemp[1])
					m_GPUTempIcon.SetShow(false);
				if(hasRAMTemp[1])
					m_RAMTempIcon.SetShow(false);
				if(hasMBTemp[1])
					m_MBTempIcon.SetShow(false);
			}
		}
	}
}



void CScnSysInfo::SetIp(void)
{
	if(hasIPAddress[0])
	{
		if (SETTINGS::getInstance().getShowIP() == 1) 
		{
			string ip = StatusMonitor::getInstance().GetIP();
			m_IPAddress.SetText(strtowstr(ip).c_str());
			
			if(hasIPAddress[1])
				m_IPAddressIcon.SetShow(true);

		} else {
			m_IPAddress.SetText(NULL);
			if(hasIPAddress[1])
				m_IPAddressIcon.SetShow(false);

		}
	}
}

void CScnSysInfo::FillStorageDevices( void )
{

	deviceHDD1.devicePath = "Hdd1:";
	deviceMU0.devicePath = "Memunit0:";
	deviceUSB0.devicePath = "Usb0:";
	deviceUSB1.devicePath = "Usb1:";

	deviceHDD1.deviceLabel = "HDD1: ";
	deviceMU0.deviceLabel = "MU0: ";
	deviceUSB0.deviceLabel = "USB0: ";
	deviceUSB1.deviceLabel = "USB1: ";

	// Set Display Type (ie., Used Space vs Free Space)
	deviceHDD1.nDisplayType = nDisplayType;
	deviceMU0.nDisplayType = nDisplayType;
	deviceUSB0.nDisplayType = nDisplayType;
	deviceUSB1.nDisplayType = nDisplayType;

	// Set Percentage Type (ie., Percent Used vs Percent Free)
	deviceHDD1.nPercentageType = nPercentageType;
	deviceMU0.nPercentageType = nPercentageType;
	deviceUSB0.nPercentageType = nPercentageType;
	deviceUSB1.nPercentageType = nPercentageType;

	// Set showLabel Flag
	deviceHDD1.showLabel = !bOverrideStorageLabel;
	deviceMU0.showLabel = !bOverrideStorageLabel;
	deviceUSB0.showLabel = !bOverrideStorageLabel;
	deviceUSB1.showLabel = !bOverrideStorageLabel;

	// Set showUnits Flag
	deviceHDD1.showUnits = !bOverrideStorageUnits;
	deviceMU0.showUnits = !bOverrideStorageUnits;
	deviceUSB0.showUnits = !bOverrideStorageUnits;
	deviceUSB1.showUnits = !bOverrideStorageUnits;

	// Set UnitLocation (ie., 10.0GB/150 or 10.0/150GB)
	deviceHDD1.nUnitLocation = nUnitLocation;
	deviceMU0.nUnitLocation = nUnitLocation;
	deviceUSB0.nUnitLocation = nUnitLocation;
	deviceUSB1.nUnitLocation = nUnitLocation;

	// Set Unmounted Text (ie., "Drive Not Mounted")
	deviceHDD1.strUnmountedText = sNotMountedText;
	deviceMU0.strUnmountedText = sNotMountedText;
	deviceUSB0.strUnmountedText = sNotMountedText;
	deviceUSB1.strUnmountedText = sNotMountedText;

	// Set showUnmountedText flag
	deviceHDD1.showUnmountedText = bUseUnmountedText;
	deviceMU0.showUnmountedText = bUseUnmountedText;
	deviceUSB0.showUnmountedText = bUseUnmountedText;
	deviceUSB1.showUnmountedText = bUseUnmountedText;

	// Set showIcon Flag
	deviceHDD1.showIcon = !bDisableStorageIcons;
	deviceMU0.showIcon = !bDisableStorageIcons;
	deviceUSB0.showIcon = !bDisableStorageIcons;
	deviceUSB1.showIcon = !bDisableStorageIcons;

	// Set showGraph Flag
	deviceHDD1.showGraph = bShowGraphUnmounted;
	deviceMU0.showGraph = bShowGraphUnmounted;
	deviceUSB0.showGraph = bShowGraphUnmounted;
	deviceUSB1.showGraph = bShowGraphUnmounted;
	
	// Set Free Space, Used Space, Total Space, Units, Percentage Value, System Path, and Label
	GetDriveInformation(&deviceHDD1);
	GetDriveInformation(&deviceMU0);
	GetDriveInformation(&deviceUSB0);
	GetDriveInformation(&deviceUSB1);

}

void CScnSysInfo::HideAllStorageControls()
{
	if(deviceHDD1.hasText)
		m_HDD1Text.SetText(NULL);
	if(deviceHDD1.hasTotal)
		m_HDD1Total.SetText(NULL);
	if(deviceHDD1.hasIcon)
		m_HDD1TextIcon.SetShow(false);
	if(deviceHDD1.hasProgress)
		m_HDD1ProgressBar.SetShow(false);

	if(deviceMU0.hasText)
		m_MU0Text.SetText(NULL);
	if(deviceMU0.hasTotal)
		m_MU0Total.SetText(NULL);
	if(deviceMU0.hasIcon)
		m_MU0TextIcon.SetShow(false);
	if(deviceMU0.hasProgress)
		m_MU0ProgressBar.SetShow(false);

	if(deviceUSB0.hasText)
		m_USB0Text.SetText(NULL);
	if(deviceUSB0.hasTotal)
		m_USB0Total.SetText(NULL);
	if(deviceUSB0.hasIcon)
		m_USB0TextIcon.SetShow(false);
	if(deviceUSB0.hasProgress)
		m_USB0ProgressBar.SetShow(false);

	if(deviceUSB1.hasText)
		m_USB1Text.SetText(NULL);
	if(deviceUSB1.hasTotal)
		m_USB1Total.SetText(NULL);
	if(deviceUSB1.hasIcon)
		m_USB1TextIcon.SetShow(false);
	if(deviceUSB1.hasProgress)
		m_USB1ProgressBar.SetShow(false);

}

string CScnSysInfo::CreateDisplayString(bool isTotal, StorageDevice device)
{
	float fDisplayValue;

	string strDisplayUnits;
	string strDisplayValue;
	string strDisplayLabel;

	string strFinalResult;

	if(!isTotal)  // we're looking at the free/used space text
	{
		if(device.nDisplayType == SO_FREE)
			fDisplayValue = device.fFree;
		else
			fDisplayValue = device.fUsed;

		strDisplayValue = sprintfaA("%0.1f", fDisplayValue);

		switch (device.nUnitLocation)
		{
		case SO_UNITBOTH:
		case SO_UNITFIRST:
			strDisplayUnits = device.sizeUnits;
			break;
		case SO_UNITNONE:
		case SO_UNITSECOND:
			strDisplayUnits = "";
			break;
		};

		if(!device.showUnits)
			strDisplayUnits = "";

		strDisplayLabel = device.deviceLabel;
		
		if(!device.showLabel)
			strDisplayLabel = "";

		strFinalResult = strDisplayLabel + strDisplayValue + strDisplayUnits;
	}else{ // we're looking at the total space text

		strDisplayValue = sprintfaA("%0.1f", device.fTotal);

		switch (device.nUnitLocation)
		{
		case SO_UNITBOTH:
		case SO_UNITSECOND:
			strDisplayUnits = device.sizeUnits;
			break;
		case SO_UNITNONE:
		case SO_UNITFIRST:
			strDisplayUnits = "";
			break;
		};

		if(!device.showUnits)
			strDisplayUnits = "";

		strFinalResult = strDisplayValue + strDisplayUnits;
	}

	return strFinalResult;
}

void CScnSysInfo::GetDriveInformation(StorageDevice * device)
{

	if(device->devicePath.length() != 0)
	{
		//Fill device struct with sizeinformation and appropriate units
		GetStorageSizeInfo(device);
		
		//determine what percentage we want and tell store the value * 10
		if(device->nPercentageType == SO_USEDPERCENT)
		{
			device->nPercentage = (int) ((device->fUsed / device->fTotal) * 1000.0f);
		}else
		{
			device->nPercentage = (int) ((device->fFree / device->fTotal) * 1000.0f);
		}
	}
}


void CScnSysInfo::UpdateStorageInfo( void )
{
	if(SETTINGS::getInstance().getDiskStatus() == 1)
	{
		FillStorageDevices();
		
		SetStorageElements(deviceHDD1, &m_HDD1Text, &m_HDD1Total, &m_HDD1TextIcon, &m_HDD1ProgressBar);
		SetStorageElements(deviceMU0, &m_MU0Text, &m_MU0Total, &m_MU0TextIcon, &m_MU0ProgressBar);
		SetStorageElements(deviceUSB0, &m_USB0Text, &m_USB0Total, &m_USB0TextIcon, &m_USB0ProgressBar);
		SetStorageElements(deviceUSB1, &m_USB1Text, &m_USB1Total, &m_USB1TextIcon, &m_USB1ProgressBar);
	}else
	{
		HideAllStorageControls();
	}
}

void CScnSysInfo::SetStorageElements(StorageDevice device, CXuiTextElement * m_firstElement, CXuiTextElement * m_secondElement, CXuiImageElement * m_imageElement, CXuiProgressBar * m_progressElement)
{
	string strFirstElement;
	string strSecondElement;
	Drive* drive = DrivesManager::getInstance().getDriveByMountPoint(device.devicePath);

	if(drive != NULL && drive->IsMounted())
	{

		strFirstElement = CreateDisplayString(false, device);
		strSecondElement = CreateDisplayString(true, device);

		if(device.hasIcon)
			m_imageElement->SetShow(true);
		if(device.hasProgress)
		{
			m_progressElement->SetShow(true);
			m_progressElement->SetRange(0, 1000);
			m_progressElement->SetValue(device.nPercentage);
		}
		if(device.hasText)
			m_firstElement->SetText(strtowstr(strFirstElement).c_str());

		if(device.hasText)
			m_secondElement->SetText(strtowstr(strSecondElement).c_str());

	}
	else
	{
		if(device.showUnmountedText)
		{
			strFirstElement = device.strUnmountedText;
			strSecondElement = "";
			
			if(device.hasIcon)
				m_imageElement->SetShow(true);
			if(device.hasProgress)
			{
				m_progressElement->SetRange(0, 1000);
				m_progressElement->SetValue(0);
				if(device.showGraph)
					m_progressElement->SetShow(true);
				else
					m_progressElement->SetShow(false);
			}
			if(device.hasText)
				m_firstElement->SetText(strtowstr(strFirstElement).c_str());
			if(device.hasTotal)
				m_secondElement->SetText(NULL);
		}else
		{
			if(device.hasIcon)
			{
				if(device.showIcon)
					m_imageElement->SetShow(true);
				else
					m_imageElement->SetShow(false);
			}
			if(device.hasProgress)
			{
				if(device.showGraph)
					m_progressElement->SetShow(true);
				else
					m_progressElement->SetShow(false);
			}
			if(device.hasText)
				m_firstElement->SetText(NULL);

			if(device.hasTotal)
				m_secondElement->SetText(NULL);
		}

	}	
}

void CScnSysInfo::GetStorageSizeInfo(StorageDevice * device)
{	
	bool bGBFlag;
	string file;
	Drive* curDrive = DrivesManager::getInstance().getDriveByMountPoint(device->devicePath);
//	file = device->devicePath + "\\";

/*	ULARGE_INTEGER FreeBytesAvailable;
	ULARGE_INTEGER TotalNumberOfBytes;
	ULARGE_INTEGER TotalNumberOfFreeBytes;*/

	//GetDiskFreeSpaceEx(file.c_str(),&FreeBytesAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);

	if(curDrive->getTotalNumberOfBytes() > 1024*1024*1024)
		bGBFlag = true;
	else
		bGBFlag = false;

	if(curDrive!=NULL)
	{
		DebugMsg("ScnSysInfo","Get Space for %s",device->devicePath.c_str());
		if(bGBFlag)
		{
			device->fFree = (float)(curDrive->getFreeNumberOfBytes()/(1024.0f * 1024.0f * 1024.0f));
			device->fTotal = (float)(curDrive->getTotalNumberOfBytes()/(1024.0f * 1024.0f * 1024.0f));
			device->fUsed = device->fTotal - device->fFree;
			device->sizeUnits = "GB";
		}else{
			device->fFree = (float)(curDrive->getFreeNumberOfBytes()/(1024.0f * 1024.0f));
			device->fTotal = (float)(curDrive->getTotalNumberOfBytes()/(1024.0f * 1024.0f));
			device->fUsed = device->fTotal - device->fFree;
			device->sizeUnits = "MB";
		}
	}
	else
	{
		DebugMsg("ScnSysInfo","Drive %s is NULL",device->devicePath.c_str());
	}

}

void CScnSysInfo::UpdateRamStruct(RAMStructure * ramstruct)
{
	MEMORYSTATUS stat;
	GlobalMemoryStatus(&stat);

	ramstruct->availableRam = stat.dwAvailPhys;
	ramstruct->totalRam = stat.dwTotalPhys;
	ramstruct->utilizedRam = stat.dwTotalPhys - stat.dwAvailPhys;
}

void CScnSysInfo::SetMemoryText( void )
{

	RAMStructure ramStruct;
	string strAvailableRam, strUtilizedRam, strTotalRam;

	if(SETTINGS::getInstance().getShowStatus() == 1)
	{
		UpdateRamStruct(&ramStruct);
		strAvailableRam = sprintfaA("%s", ramStruct.availableRam/1024/1024);
		strUtilizedRam = sprintfaA("%s", ramStruct.utilizedRam/1024/1024);
		strTotalRam = sprintfaA("%s", ramStruct.totalRam/1024/1024);

		if(hasAvailableRam[0])
		{
			m_AvailableRam.SetShow(true);
			if(hasAvailableRam[1])
				m_AvailableRamIcon.SetShow(true);

			m_AvailableRam.SetText(strtowstr(strAvailableRam).c_str());			
		}
		if(hasUtilizedRam[0])
		{	
			m_UtilizedRam.SetShow(true);
			if(hasUtilizedRam[1])
				m_UtilizedRamIcon.SetShow(true);
			
			m_UtilizedRam.SetText(strtowstr(strUtilizedRam).c_str());
		}
		if(hasTotalRam[0])
		{
			m_TotalRam.SetShow(true);
			if(hasTotalRam[1])
				m_TotalRamIcon.SetShow(true);
			
			m_TotalRam.SetText(strtowstr(strTotalRam).c_str());
		}
	}else
	{
		if(hasAvailableRam[0])
			m_AvailableRam.SetText(NULL);

		if(hasUtilizedRam[0])
			m_UtilizedRam.SetText(NULL);

		if(hasTotalRam[0])
			m_TotalRam.SetText(NULL);

		if(hasAvailableRam[1])
			m_AvailableRamIcon.SetShow(false);
	
		if(hasUtilizedRam[1])
			m_UtilizedRamIcon.SetShow(false);

		if(hasTotalRam[1])
			m_TotalRamIcon.SetShow(false);
	}
}
	

void CScnSysInfo::UpdateSMCStructure(bool initialRun)
{
	smc MySMC;	
	

	if(hasSMC[0] && initialRun)
		smcStruct.SMCVersion = sprintfaA("%s", MySMC.GetSMCVersion());

	if(hasTilt[0])
		smcStruct.TiltState = sprintfaA("%s", (MySMC.GetTiltState() == VERTICAL ? "Vertical" : "Horizontal"));

	if(hasAVPack[0] && initialRun)
	{
		const char * AVPackName = MySMC.GetAVPackName();
		string strAVPack;
		
		if(AVPackName)
			strAVPack = sprintfaA("%s", MySMC.GetAVPackName());
		else
			strAVPack = sprintfaA("0x%02", MySMC.GetAVPack());	
		
		smcStruct.AVPack = strAVPack;
	}

	if(hasTrayState[0])
	{
		
		char * TrayStates[] = {"Open", "-", "Closed", "Opening", "Closing"};
		string strTrayState = sprintfaA("%s", TrayStates[MySMC.GetTrayState()]);

		smcStruct.TrayState = strTrayState;
	}
}

					
		

void CScnSysInfo::SetInfoText( void )
{

	if(SETTINGS::getInstance().getShowStatus() == 1)
	{
		if(hasSMC[0])
		{	
			m_SMCVer.SetText(strtowstr(smcStruct.SMCVersion).c_str());
			m_SMCVer.SetShow(true);
			if(hasSMC[1])
				m_SMCVerIcon.SetShow(true);
		}
		if(hasTilt[0])
		{	
			m_Tilt.SetText(strtowstr(smcStruct.TiltState).c_str());
			m_Tilt.SetShow(true);
			if(hasTilt[1])
				m_TiltIcon.SetShow(true);
		}
		if(hasTrayState[0])
		{	
			m_TrayState.SetText(strtowstr(smcStruct.TrayState).c_str());
			m_TrayState.SetShow(true);
			if(hasTrayState[1])
				m_TrayStateIcon.SetShow(true);
		}
		if(hasAVPack[0])
		{	
			m_AVPack.SetText(strtowstr(smcStruct.AVPack).c_str());
			m_AVPack.SetShow(true);
			if(hasAVPack[1])
				m_AVPackIcon.SetShow(true);
		}
	}else
	{
		if(hasSMC[0])
			m_SMCVer.SetText(NULL);
		if(hasTilt[0])
			m_Tilt.SetText(NULL);
		if(hasTrayState[0])
			m_TrayState.SetText(NULL);
		if(hasAVPack[0])
			m_AVPack.SetText(NULL);

		if(hasSMC[1])
			m_SMCVerIcon.SetShow(false);
		if(hasTilt[1])
			m_TiltIcon.SetShow(false);
		if(hasTrayState[1])
			m_TrayStateIcon.SetShow(false);
		if(hasAVPack[1])
			m_AVPackIcon.SetShow(false);
	}
}


HRESULT CScnSysInfo::InitializeChildren( void )
{
	HRESULT hr = NULL;
	
	if(showDebugMsg)
		DebugMsg("ScnSysInfo","CScnSysInfo::InitializeChildren");
	
	hr = GetChildById( L"textDate", &m_date );
	hasDate[0] = hr == S_OK;
	hr = GetChildById( L"iconDate", &m_dateIcon );
	hasDate[1] = hr == S_OK;

	hr = GetChildById( L"textTime", &m_time );
	hasTime[0] = hr == S_OK;
	hr = GetChildById( L"iconTime", &m_timeIcon );
	hasTime[1] = hr == S_OK;

	hr = GetChildById( L"textDateTime", &m_datetime );
	hasDateTime[0] = hr == S_OK;
	hr = GetChildById( L"iconDateTime", &m_datetimeIcon );
	hasDateTime[1] = hr == S_OK;

	hr = GetChildById( L"textCPUTemp", &m_CPUTemp );
	hasCPUTemp[0] = hr == S_OK;
	hr = GetChildById( L"iconCPUTemp", &m_CPUTempIcon );
	hasCPUTemp[1] = hr == S_OK;
	hr = GetChildById( L"textGPUTemp", &m_GPUTemp );
	hasGPUTemp[0] = hr == S_OK;
	hr = GetChildById( L"iconGPUTemp", &m_GPUTempIcon );
	hasGPUTemp[1] = hr == S_OK;
	hr = GetChildById( L"textMBTemp", &m_MBTemp );
	hasMBTemp[0] = hr == S_OK;
	hr = GetChildById( L"iconMBTemp", &m_MBTempIcon );
	hasMBTemp[1] = hr == S_OK;
	hr = GetChildById( L"textRAMTemp", &m_RAMTemp );
	hasRAMTemp[0] = hr == S_OK;
	hr = GetChildById( L"iconRAMTemp", &m_RAMTempIcon );
	hasRAMTemp[1] = hr == S_OK;
	hr = GetChildById( L"CurrentStatus", &m_CurrentStatus);
	hasCurrentStatus = hr == S_OK;
	hr = GetChildById( L"textIPAddress", &m_IPAddress );
	hasIPAddress[0] = hr == S_OK;
	hr = GetChildById( L"iconIPAddress", &m_IPAddressIcon );
	hasIPAddress[1] = hr == S_OK;

	hr = GetChildById( L"textHDD1", &m_HDD1Text );
	deviceHDD1.hasText = hr == S_OK;
	hr = GetChildById( L"iconHDD1", &m_HDD1TextIcon );
	deviceHDD1.hasIcon = hr == S_OK;
	hr = GetChildById( L"textMU0", &m_MU0Text );
	deviceMU0.hasText = hr == S_OK;
	hr = GetChildById( L"iconMU0", &m_MU0TextIcon );
	deviceMU0.hasIcon = hr == S_OK;
	hr = GetChildById( L"textUSB0", &m_USB0Text );
	deviceUSB0.hasText = hr == S_OK;
	hr = GetChildById( L"iconUSB0", &m_USB0TextIcon );
	deviceUSB0.hasIcon = hr == S_OK;
	hr = GetChildById( L"textUSB1", &m_USB1Text );
	deviceUSB1.hasText = hr == S_OK;
	hr = GetChildById( L"iconUSB1", &m_USB1TextIcon );
	deviceUSB1.hasIcon = hr == S_OK;

	hr = GetChildById( L"textSMC", &m_SMCVer );
	hasSMC[0] = hr == S_OK;
	hr = GetChildById( L"iconSMC", &m_SMCVerIcon );
	hasSMC[1] = hr == S_OK;
	hr = GetChildById( L"textTilt", &m_Tilt );
	hasTilt[0] = hr == S_OK;	
	hr = GetChildById( L"iconTilt", &m_TiltIcon );
	hasTilt[1] = hr == S_OK;
	hr = GetChildById( L"textTrayState", &m_TrayState );
	hasTrayState[0] = hr == S_OK;
	hr = GetChildById( L"iconTrayState", &m_TrayStateIcon );
	hasTrayState[1] = hr == S_OK;
	hr = GetChildById( L"textAVPack", &m_AVPack );
	hasAVPack[0] = hr == S_OK;
	hr = GetChildById( L"iconAVPack", &m_AVPackIcon );
	hasAVPack[1] = hr == S_OK;

	hr = GetChildById( L"textFreeRam", &m_AvailableRam );
	hasAvailableRam[0] = hr == S_OK;
	hr = GetChildById( L"iconFreeRam", &m_AvailableRamIcon );
	hasAvailableRam[1] = hr == S_OK;
	hr = GetChildById( L"textUsedRam", &m_UtilizedRam );
	hasUtilizedRam[0] = hr == S_OK;
	hr = GetChildById( L"iconUsedRam", &m_UtilizedRamIcon );
	hasUtilizedRam[1] = hr == S_OK;
	hr = GetChildById( L"textTotalRam", &m_TotalRam );
	hasTotalRam[0] = hr == S_OK;
	hr = GetChildById( L"iconTotalRam", &m_TotalRamIcon );
	hasTotalRam[1] = hr == S_OK;

	hr = GetChildById( L"CPUGraph", &m_CPUProgressBar );
	hasProgressBar[CPU] = hr == S_OK;
	hr = GetChildById( L"GPUGraph", &m_GPUProgressBar );
	hasProgressBar[GPU] = hr == S_OK;
	hr = GetChildById( L"RAMGraph", &m_RAMProgressBar );
	hasProgressBar[MEM] = hr == S_OK;
	hr = GetChildById( L"MBGraph", &m_MBProgressBar );
	hasProgressBar[BRD] = hr == S_OK;

	hr = GetChildById( L"HDD1Graph", &m_HDD1ProgressBar);
	deviceHDD1.hasProgress = hr == S_OK;
	hr = GetChildById( L"MU0Graph", &m_MU0ProgressBar);
	deviceMU0.hasProgress = hr == S_OK;
	hr = GetChildById( L"USB0Graph", &m_USB0ProgressBar);
	deviceUSB0.hasProgress = hr == S_OK;
	hr = GetChildById( L"USB1Graph", &m_USB1ProgressBar);
	deviceUSB1.hasProgress = hr == S_OK;

	hr = GetChildById( L"HDD1Total", &m_HDD1Total);
	deviceHDD1.hasTotal = hr == S_OK;
	hr = GetChildById( L"MU0Total", &m_MU0Total);
	deviceMU0.hasTotal = hr == S_OK;
	hr = GetChildById( L"USB0Total", &m_USB0Total);
	deviceUSB0.hasTotal = hr == S_OK;
	hr = GetChildById( L"USB1Total", &m_USB1Total);
	deviceUSB1.hasTotal = hr == S_OK;

	return S_OK;
}