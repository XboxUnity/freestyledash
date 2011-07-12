#pragma once

#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../../Tools/Generic/tools.h"
#include "../../Tools/SMC/smc.h"
#include "../../Tools/TextScroller/TextScroller.h"


using namespace std;

typedef struct
{
	int nUnitLocation;
	int nPercentage;
	int nPercentageType;
	int nDisplayType;
	
	float fFree;
	float fUsed;
	float fTotal;
	
	string devicePath;
	string deviceLabel;
	string sizeUnits;
	string strUnmountedText;

	bool hasIcon;
	bool hasProgress;
	bool hasTotal;
	bool hasText;
	bool showUnits;
	bool showLabel;
	bool showIcon;
	bool showGraph;
	bool showUnmountedText;
	
}StorageDevice;

typedef struct
{
	SIZE_T availableRam;
	SIZE_T totalRam;
	SIZE_T utilizedRam;

}RAMStructure;

typedef struct
{
	string SMCVersion;
	string TiltState;
	string TrayState;
	string AVPack;
}SMCInfo;


enum StorageOptions {
	SO_FREE,
	SO_USED,
	SO_TOTAL,
	SO_UNITFIRST,
	SO_UNITSECOND,
	SO_UNITNONE,
	SO_UNITBOTH,
	SO_USEDPERCENT,
	SO_FREEPERCENT
};

class CScnSysInfo : public ConfigurableScene
{

protected:
	bool fastMode;
	bool storageLoadedOnce;
	bool showDebugMsg;  //set to false to hide debug messages for this class

	CXuiTextElement m_date; CXuiImageElement m_dateIcon;
	CXuiTextElement m_time; CXuiImageElement m_timeIcon;
	CXuiTextElement m_datetime; CXuiImageElement m_datetimeIcon;
	CXuiTextElement m_CurrentStatus;
	CXuiTextElement m_MBTemp; CXuiImageElement m_MBTempIcon;
	CXuiTextElement m_CPUTemp; CXuiImageElement m_CPUTempIcon;
	CXuiTextElement m_GPUTemp; CXuiImageElement m_GPUTempIcon;
	CXuiTextElement m_RAMTemp; CXuiImageElement m_RAMTempIcon;

	CXuiTextElement m_IPAddress; CXuiImageElement m_IPAddressIcon;

	CXuiTextElement m_HDD1Text; CXuiImageElement m_HDD1TextIcon;
	CXuiTextElement m_MU0Text; CXuiImageElement m_MU0TextIcon;
	CXuiTextElement m_USB0Text; CXuiImageElement m_USB0TextIcon;
	CXuiTextElement m_USB1Text; CXuiImageElement m_USB1TextIcon;

	CXuiTextElement m_SMCVer; CXuiImageElement m_SMCVerIcon;
	CXuiTextElement m_Tilt; CXuiImageElement m_TiltIcon;
	CXuiTextElement m_AVPack; CXuiImageElement m_AVPackIcon;
	CXuiTextElement m_TrayState; CXuiImageElement m_TrayStateIcon;

	CXuiTextElement m_AvailableRam; CXuiImageElement m_AvailableRamIcon;
	CXuiTextElement m_UtilizedRam; CXuiImageElement m_UtilizedRamIcon;
	CXuiTextElement m_TotalRam; CXuiImageElement m_TotalRamIcon;

	CXuiProgressBar m_CPUProgressBar, m_GPUProgressBar, m_RAMProgressBar, m_MBProgressBar;
	CXuiProgressBar m_HDD1ProgressBar, m_MU0ProgressBar, m_USB0ProgressBar, m_USB1ProgressBar;
	
	CXuiTextElement m_HDD1Total, m_MU0Total, m_USB0Total, m_USB1Total;
	
	bool hasProgressBar[4];
	bool hasCurrentStatus;
	StorageDevice deviceHDD1, deviceMU0, deviceUSB0, deviceUSB1;
	SMCInfo smcStruct;

	string sNotMountedText;
	int nDisplayType, nPercentageType, nUnitLocation;
	bool bOverrideStorageLabel, bOverrideStorageUnits;
	bool bDisableStorageIcons, bUseUnmountedText; 
	bool bShowGraphUnmounted;

	// variable to determine if control has been created on scene (0 = textbox, 1 = icon)
	bool hasDate[2], hasTime[2], hasDateTime[2];
	bool hasMBTemp[2], hasCPUTemp[2], hasGPUTemp[2], hasRAMTemp[2];
	bool hasIPAddress[2];
	bool hasSMC[2], hasTilt[2], hasAVPack[2], hasTrayState[2];
	bool hasAvailableRam[2], hasUtilizedRam[2], hasTotalRam[2];

	bool bOverrideTempLabel, bOverrideTempUnit;
	bool bDisableTempIcons, bDisableIPAddrIcons;

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	void CScnSysInfo::SetCurrentStatus(void);
	void CScnSysInfo::SetInfoText( void );
	void CScnSysInfo::SetIp(void);
	void CScnSysInfo::SetTemps( void ); 
	void CScnSysInfo::SetClock( void );

	void CScnSysInfo::ParseSettings( void );
	void CScnSysInfo::GetDriveInformation(StorageDevice * device);
	void CScnSysInfo::GetStorageSizeInfo(StorageDevice * device);

	void CScnSysInfo::SetStorageElements(StorageDevice device, CXuiTextElement * m_firstElement, CXuiTextElement * m_secondElement, CXuiImageElement * m_imageElement, CXuiProgressBar * m_progressElement);
	void CScnSysInfo::FillStorageDevices( void );
	void CScnSysInfo::UpdateStorageInfo( void );
	void CScnSysInfo::HideAllStorageControls();
	void CScnSysInfo::UpdateSMCStructure(bool initialRun);
	void CScnSysInfo::SetMemoryText( void );
	void CScnSysInfo::UpdateRamStruct(RAMStructure * ramstruct);
	
	string CScnSysInfo::CreateDisplayString(bool isTotal, StorageDevice device);
	HRESULT CScnSysInfo::InitializeChildren( void );
	float CScnSysInfo::UpdateTempGraphs( int deviceType);
	string CScnSysInfo::GetIp( void );

public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnSysInfo, L"ScnSysInfo", XUI_CLASS_SCENE )
};
