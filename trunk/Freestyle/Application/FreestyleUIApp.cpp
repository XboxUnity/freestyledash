#pragma once
#include "stdafx.h"
#include "FreestyleApp.h"
#include "FreestyleUIApp.h"

// Utilities
#include "../Tools/Threads/LoadingThread.h"
#include "../Tools/Managers/Skin/SkinManager.h"
#include "../Tools/HTTPServer/HTTPServer.h"
#include "../Tools/Managers/Theme/CanvasManager/CanvasManager.h"
#include "../Tools/Managers/Theme/TabManager/TabManager.h"
#include "../Tools/DVDInfo/DVDItem.h"
#include "../Tools/DVDInfo/DVDWorker.h"

// Standard XUI Scenes
#include "../Scenes/Popup/ScnPopupWindow.h"
#include "../Scenes/Profile/ScnProfile.h"
#include "../Scenes/SystemInfo/ScnSystemInfo.h"
#include "../Scenes/Main/ScnMain.h"
#include "../Scenes/Utilities/ScnUtilities.h"
#include "../Scenes/FileBrowser/FilesList/FilesList.h"
#include "../Scenes/DualPane/DestFileList/DestFileList.h"
#include "../Scenes/DualPane/SourceFileList/SourceFileList.h"
#include "../Scenes/FileBrowser/ScnFileBrowser.h"
#include "../Scenes/FileBrowser/ScnFileOp.h"
#include "../Scenes/ChooseSkin/ScnChooseSkin.h"
#include "../Scenes/Options/ScnOptionsGeneralSettings.h"
#include "../Scenes/Options/ScnOptionsContentSettings.h"
#include "../Scenes/Options/ScnOptionsSystemSettings.h"
#include "../Scenes/Options/ScnOptionsXlinkKaiSettings.h"
#include "../Scenes/Rss/ScnRssReader.h"
#include "../Scenes/GameView/GameList/GamesList.h"
#include "../Scenes/About/ScnAbout.h"
#include "../Scenes/Theme/Presenters/Tab/ScnTabPresenter.h"
#include "../Scenes/Theme/Presenters/Canvas/ScnCanvasPresenter.h"
#include "../Scenes/Theme/Config/Canvas/ScnCanvasConfig.h"
#include "../Scenes/Theme/Config/Tab/ScnTabConfig.h"
#include "../Scenes/GameView/ScnGameView.h"
#include "../Scenes/CopyDVD/ScnCopyDVD.h"
#include "../Scenes/GameSaves/GameSaveList/SavedGamesList.h"
#include "../Scenes/GameSaves/ScnGameSaves.h"
#include "../Scenes/Updater/ScnUpdater.h"
#include "../Scenes/GameControls/ScnGameControls.h"
#include "../Scenes/DualPane/ScnDualPane.h"
#include "../Scenes/ManageTUs/ScnManageTUs.h"
#include "../Scenes/ManageTUs/ActiveList/ActiveList.h"
#include "../Scenes/ManageTUs/PathList/TUPaths.h"
#include "../Scenes/Screenshots/ScnScreenshots.h"
#include "../Scenes/Screenshots/ScreenshotList/ScreenshotList.h"
#include "../Scenes/ManagePaths/ScnManagePaths.h"
#include "../Scenes/ManagePaths/PathList/PathList.h"
#include "../Scenes/TeamFSD/ScnTeamFSD.h"
#include "../Scenes/Achievements/ScnAchievements.h"
#include "../Scenes/Achievements/AchievementList/AchievementList.h"
#include "../Scenes/Wait/ScnWait.h"
#include "../Scenes/Options/ScnOptionsMain.h"
#include "../Scenes/Options/ScnParentGeneralSettings.h"
#include "../Scenes/Options/ScnParentContentSettings.h"
#include "../Scenes/Options/ScnParentSystemSettings.h"
#include "../Scenes/Options/ScnParentKaiSettings.h"
#include "../Scenes/AvatarRenderer/ScnAvatar.h"
#include "../Scenes/GetCredentials/ScnGetCredentials.h"

// XlinkKai Scenes
#include "../Scenes/XlinkKai/ScnXlinkKaiArena.h"
#include "../Scenes/XlinkKai/XlinkKaiArenaList.h"
#include "../Scenes/XlinkKai/XlinkKaiPlayerList.h"
#include "../Scenes/XlinkKai/XlinkKaiBuddyList.h"
#include "../Scenes/XlinkKai/XlinkKaiChatPlayerList.h"
#include "../Scenes/XlinkKai/ScnXlinkKaiChat.h"
#include "../Scenes/XlinkKai/ScnXlinkKai.h"
#include "../Scenes/XlinkKai/ScnXlinkSignIn.h"
#include "../Scenes/XlinkKai/ScnXlinkKaiHost.h"
#include "../Scenes/XlinkKai/ScnXlinkKaiPlayerInfo.h"

// Plugin Scenes
#include "../Scenes/Plugin/ScnPlugin.h"
#include "../Scenes/Plugin/Settings/ScnPluginSettings.h"
#include "../Scenes/Plugin/Trainers/ScnPluginTrainers.h"
#include "../Scenes/Plugin/Trainers/TrainerList/TrainerList.h"
#include "../Scenes/Plugin/Trainers/OptionList/TrainerOptionList.h"
#include "../Scenes/Plugin/Screenshots/ScnPluginScreenshots.h"

// Weather Scenes
#include "../Scenes/Weather/ScnWeather.h"
#include "../Scenes/Weather/ScnWeatherLocation.h"
#include "../Scenes/Weather/LocationList/LocationList.h"

HRESULT CFreestyleUIApp::InitializeUI( void * pInitData )
{
	HRESULT retVal = NULL;
	if(SETTINGS::getInstance().getInstallerState() == 0)
	{
		retVal = InitShared( CFreestyleApp::getInstance().m_pd3dDevice, &(CFreestyleApp::getInstance().m_d3dpp), XuiTextureLoader );
		if( FAILED( retVal ) )
			return retVal;

		retVal = XuiRenderGetXuiDevice(GetDC(), &m_pXuiDevice);
		if(retVal != S_OK)
			m_pXuiDevice = NULL;

	}
	CFreestyleApp::getInstance().InitializeVersionInfo();

	//Detect if Installer is Schedueld to Run
	if(SETTINGS::getInstance().getInstallerState() > 0)
	{
		//XuiRenderUninit();
		//Uninit();
		//m_pXuiDevice->Release();
		//CFreestyleApp::getInstance().m_OverlayFont.Destroy();
		AutoUpdater::getInstance().RunInstaller();
	}
	
	// Check for Update
	if(CFreestyleApp::getInstance().hasInternetConnection()) {
		if(SETTINGS::getInstance().getUpdatePluginNotice() == 1 ||
			SETTINGS::getInstance().getUpdateSkinNotice() == 1 ||
			SETTINGS::getInstance().getUpdateXexNotice() == 1 )	
		{
			
			AutoUpdater::getInstance().CheckForUpdates(true);
		}
	}

	// Register the default font and call it ConsoleFont
	retVal = RegisterDefaultTypeface( L"ConsoleFont", L"file://flash:\\xenonjklatin.xtt" );
	if( FAILED( retVal ) )
		return retVal;
	
	// Determine the first scene to load based on user's settings
	if(SETTINGS::getInstance().getDataPath().empty()) {
		SETTINGS::getInstance().setDataPath("game:\\Data\\", true);
	}

	DebugMsg("CFreestyleApp", "Loading Skin File:  Main.Xur" );
	SkinManager::getInstance().setSkin( *this, "main.xur", false );

	CanvasManager::getInstance();
	TabManager::getInstance();

	if(CFreestyleApp::getInstance().getFirstRun() == true)
	{
		string szMessage = sprintfaA("Welcome to Freestyle Dash %d.%d %s", 
			VersionManager::getInstance().getFSDDashVersion().Version.dwMajor,
			VersionManager::getInstance().getFSDDashVersion().Version.dwMinor,
			VersionManager::getInstance().ConvertTypeToString( VersionManager::getInstance().getFSDDashVersion().Version.dwVersionType, false).c_str()
		);

		XNotifyQueueUICustom(strtowchar(szMessage));
	}

	// Installer isnt Running, so let's go ahead and start our content loading
	m_LoadingThread.CreateThread( CPU2_THREAD_1 );

	// Start up modules that need Skin and Content loaded to proceed
	if(CFreestyleApp::getInstance().hasInternetConnection()) {
		HTTPServer::getInstance();
	}

	// Resume UI Rendering
    Resume();

	// This will force the first trigger on a jtag system
	DWORD consoleType = 0;
	XeKeysGetConsoleTypeCustom(&consoleType);
	if(consoleType == CONSOLETYPE_RETAIL)
		DVDMonitor::getInstance().ForceFirstTrigger();

	// Return Successfully
	return S_OK;
}

HRESULT CFreestyleUIApp::UpdateUI( void * pUpdateData )
{
	// Run the next animation frame
	RunFrame();
	return S_OK;
}

HRESULT CFreestyleUIApp::PreRenderUI( void *pRenderData )
{
	// Run any XUI Related pre render objects here
	PreRender();
	return S_OK;
}

HRESULT CFreestyleUIApp::RenderUI(D3DDevice * pDevice, int nHOverscan, int nVOverscan, UINT uWidth, UINT uHeight)
{

    XuiTimersRun();
    XuiRenderBegin( GetDC(), D3DCOLOR_ARGB( 255, 0, 0, 0 ) );

    D3DXMATRIX matOrigView;
	D3DXMATRIX matOrigTrans;
    XuiRenderGetViewTransform( GetDC(), &matOrigView );

    // scale depending on the width of the render target
    D3DXMATRIX matView;
	int NewWidth = uWidth - (nHOverscan * 2);
	int NewHeight = uHeight - (nVOverscan * 2);
    D3DXVECTOR2 vScaling = D3DXVECTOR2( NewWidth / 1280.0f, NewHeight / 720.0f );
	D3DXVECTOR2 vTranslation = D3DXVECTOR2( (float)nHOverscan, (float)nVOverscan );
    D3DXMatrixTransformation2D( &matView, NULL, 0.0f, &vScaling, NULL, 0.0f, &vTranslation );
	XuiRenderSetViewTransform( GetDC(), &matView );

    XUIMessage msg;
    XUIMessageRender msgRender;
    XuiMessageRender( &msg, &msgRender, GetDC(), 0xffffffff, XUI_BLEND_NORMAL );
    XuiSendMessage( GetRootObj(), &msg );

	XuiRenderSetViewTransform( GetDC(), &matOrigView );

    XuiRenderEnd( GetDC() );

    return S_OK;
}

void CFreestyleUIApp::DispatchXuiInput( XINPUT_KEYSTROKE *pKeystroke )
{
	// Send the keystroke to the xui message system for processing
	XuiProcessInput(pKeystroke);
}

void CFreestyleUIApp::RunFrame()
{
    CXuiModule::RunFrame();
}

HRESULT CFreestyleUIApp::RegisterXuiClasses() 
{
	// Register Xui Sound and Video Systems
	XuiVideoRegister();
	XuiSoundXACTRegister();
	XuiSoundXAudioRegister();
	XuiHtmlRegister();

	// Register User Created Xui Scene Classes
	CScnMain::Register();
	CScnProfile::Register();
	CScnPopup::Register();
	CScnSysInfo::Register();
	CScnFileOp::Register();
	CScnFileBrowser::Register();
	CFilesList::Register();
	CDestFileList::Register();
	CSrcFileList::Register();
	CScnChooseSkin::Register();
	CScnRssReader::Register();
	CGamesList::Register();
	CScnAbout::Register();
	CScnTabPresenter::Register();
	CScnCanvasPresenter::Register();
	CScnCanvasConfig::Register();
	CScnTabConfig::Register();
	CScnGameView::Register();
	CScnCopyDVD::Register();
	CSavedGamesList::Register();
	CScnGameSaves::Register();
	CScnUpdater::Register();
	CScnGameControls::Register();
	CScnDualPane::Register();
	CScnManageTUs::Register();
	CActiveList::Register();
	CTUPathList::Register();
	CScnScreenshots::Register();
	CScreenshotList::Register();
	CPathList::Register();
	CScnManagePaths::Register();
	CScnTeamFSD::Register();
	CAchievementList::Register();
	CScnAchievements::Register();
	CScnWait::Register();
	CScnAvatar::Register();
	CScnUtilities::Register();

	//Setting Scenes
	CScnOptionsGeneralSettings::Register();
	CScnOptionsContentSettings::Register();
	CScnOptionsSystemSettings::Register();
	CScnOptionsXlinkKaiSettings::Register();
	CScnOptionsMain::Register();
	CScnParentGeneralSettings::Register();
	CScnParentContentSettings::Register();
	CScnParentSystemSettings::Register();
	CScnParentKaiSettings::Register();
	ScnGetCredentials::Register();

	//XlinkKai Scenes
	CScnXlinkKaiArena::Register();
	CXlinkArenaList::Register();
	CXlinkPlayerList::Register();
	CXlinkBuddyList::Register();
	CXlinkChatPlayerList::Register();
	CScnXlinkKai::Register();
	CScnXlinkKaiChat::Register();
	CScnXlinkSignIn::Register();
	CScnXlinkKaiHost::Register();
	CScnXlinkKaiPlayerInfo::Register();

	// Plugin Scenes
	CScnPlugin::Register();
	CScnPluginSettings::Register();
	CScnPluginTrainers::Register();
	CTrainerOptionList::Register();
	CTrainerList::Register();
	CScnPluginScreenshots::Register();

	// Weather Scenes
	CScnWeather::Register();
	CScnWeatherLocation::Register();
	CWeatherLocationList::Register();
    
	// Return Successfully
	return S_OK;
}

HRESULT CFreestyleUIApp::UnregisterXuiClasses()
{
	// Unregister Sound and video Systems
	XuiVideoUnregister(); 
	XuiSoundXACTUnregister();
	XuiSoundXAudioUnregister();
	XuiHtmlUnregister();

	// Unregister User Created Scene classes
	CScnMain::Unregister();
	CScnProfile::Unregister();
	CScnPopup::Unregister();
	CScnSysInfo::Unregister();
	CScnFileOp::Unregister();
	CScnFileBrowser::Unregister();
	CFilesList::Unregister();
	CDestFileList::Unregister();
	CSrcFileList::Unregister();
	CScnChooseSkin::Unregister();
	CScnRssReader::Unregister();
	CGamesList::Unregister();
	CScnAbout::Unregister();
	CScnTabPresenter::Unregister();
	CScnCanvasPresenter::Unregister();
	CScnCanvasConfig::Unregister();
	CScnTabConfig::Unregister();
	CScnGameView::Unregister();
	CScnCopyDVD::Unregister();
	CSavedGamesList::Unregister();
	CScnGameSaves::Unregister();
	CScnUpdater::Unregister();
	CScnGameControls::Unregister();
	CScnDualPane::Unregister();
	CScnManageTUs::Unregister();
	CActiveList::Unregister();
	CTUPathList::Unregister();
	CScnScreenshots::Unregister();
	CScreenshotList::Unregister();
	CPathList::Unregister();
	CScnManagePaths::Unregister();
	CScnTeamFSD::Unregister();
	CAchievementList::Unregister();
	CScnAchievements::Unregister();
	CScnWait::Unregister();
	CScnAvatar::Unregister();
	CScnUtilities::Unregister();

	//Setting Scenes
	CScnOptionsGeneralSettings::Unregister();
	CScnOptionsContentSettings::Unregister();
	CScnOptionsSystemSettings::Unregister();
	CScnOptionsXlinkKaiSettings::Unregister();
	CScnOptionsMain::Unregister();
	CScnParentGeneralSettings::Unregister();
	CScnParentContentSettings::Unregister();
	CScnParentSystemSettings::Unregister();
	CScnParentKaiSettings::Unregister();

	//XlinkKai Scenes
	CScnXlinkKaiArena::Unregister();
	CXlinkArenaList::Unregister();
	CXlinkPlayerList::Unregister();
	CXlinkBuddyList::Unregister();
	CXlinkChatPlayerList::Unregister();
	CScnXlinkKai::Unregister();
	CScnXlinkKaiChat::Unregister();
	CScnXlinkSignIn::Unregister();
	CScnXlinkKaiHost::Unregister();
	CScnXlinkKaiPlayerInfo::Unregister();

	// Plugin Scenes
	CScnPlugin::Unregister();
	CScnPluginSettings::Unregister();
	CScnPluginTrainers::Unregister();
	CTrainerOptionList::Unregister();
	CTrainerList::Unregister();
	CScnPluginScreenshots::Unregister();

	// Weather Scenes
	CScnWeather::Unregister();
	CScnWeatherLocation::Unregister();
	CWeatherLocationList::Unregister();
	
	// Return Successfully
	return S_OK;
}

HRESULT CFreestyleUIApp::CreateMainCanvas()
{
	ASSERT( m_bXuiInited );
	if( !m_bXuiInited ) {
		return E_UNEXPECTED;
	}

	ASSERT( m_hObjRoot == NULL );
	if( m_hObjRoot ) {
		return E_UNEXPECTED;
	}

	HRESULT retVal = XuiCreateObject( L"XuiCanvas", &m_hObjRoot );
	if( FAILED( retVal ) )
		return retVal;

	return retVal;
}