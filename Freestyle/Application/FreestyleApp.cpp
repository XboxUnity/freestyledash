#pragma once
#include "stdafx.h"
#include "FreestyleApp.h"
#include "FreestyleUIApp.h"

#include "../Tools/Generic/Xboxtools.h"
#include "../Tools/Debug/Debug.h"
#include "../Tools/SMC/smc.h"
#include "../Tools/Debug/Commands/ScreenshotCommand.h"
#include "../Tools/SETTINGS/Settings.h"
#include "../Tools/Managers/Theme/CanvasManager/CanvasManager.h"
#include "../Tools/Managers/Theme/TabManager/TabManager.h"
#include "../Tools/AutoUpdater/AutoUpdater.h"
#include "../Tools/FTP/FTPServer.h"
#include "../Tools/HTTP/HTTPDownloader.h"
#include "../Tools/NTP/NTP.h"
#include "../Tools/Managers/Version/VersionManager.h"
#include "../Tools/Plugin/PluginLoader.h"
#include "../Tools/Managers/SambaServer/SambaServer.h"
#include "../../Libs/libsmbd/xbox/xbLibSmb.h"

HRESULT XuiTextureLoader(IXuiDevice *pDevice, LPCWSTR szFileName, XUIImageInfo *pImageInfo, IDirect3DTexture9 **ppTex)
{
	int Len = wcslen(szFileName);
	if (szFileName[Len-3] == L'P' || szFileName[Len-3] == L'p')
		return XuiPNGTextureLoader(pDevice,szFileName,pImageInfo,ppTex);

	return XuiD3DXTextureLoader(pDevice,szFileName,pImageInfo,ppTex);
}

CFreestyleApp::CFreestyleApp( void )
{
	m_bShowFrameRate = true;
	m_bShowCPURender = true;
	m_bShowFreeRam = true;
	m_bShowVersionInfo = true;
	m_bShowProjectMode = true;

	m_bFirstRun = false;
}

HRESULT CFreestyleApp::CreateRenderTargets( void )
{
	HRESULT retVal;
	
	// Initialize D3DSurface Parameters
	D3DSURFACE_PARAMETERS ddsParams = { 0 };

	// Create RenderTarget for use with Predicated Tiling
	retVal = m_pd3dDevice->CreateRenderTarget( g_dwTileWidth, g_dwTileHeight, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pBackBuffer, &ddsParams );
	if(retVal != D3D_OK) {
		DebugMsg("CFreestyleApp", "Predicated Tiling Render Target could not be created.  HRESULT = %X", retVal);
		return S_FALSE;
	}
			
	// Set up the surface size parameters
	ddsParams.Base = m_pBackBuffer->Size / GPU_EDRAM_TILE_SIZE;
	ddsParams.HierarchicalZBase = 0;

	// Create Depth Stencil Surface
	retVal = m_pd3dDevice->CreateDepthStencilSurface( g_dwTileWidth, g_dwTileHeight, D3DFMT_D24S8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pDepthBuffer, &ddsParams );
	if(retVal != D3D_OK) {
		DebugMsg("CFreestyleApp", "Depth Stencil Surface could not be created. HRESULT = %X", retVal);
		return S_FALSE;
	}

	// Create First Frame Buffer
	retVal = m_pd3dDevice->CreateTexture( g_dwFrameWidth, g_dwFrameHeight, 1, 0, D3DFMT_LE_X8R8G8B8, 0, &m_pFrontBuffer[0], NULL );
	if(retVal != D3D_OK) {
		DebugMsg("CFreestyleApp", "Frame Buffer #1 could not be created.  HRESULT = %X", retVal);
		return S_FALSE;
	}

	// Create Second Frame Buffer
	retVal = m_pd3dDevice->CreateTexture( g_dwFrameWidth, g_dwFrameHeight, 1, 0, D3DFMT_LE_X8R8G8B8, 0, &m_pFrontBuffer[1], NULL );
	if(retVal != D3D_OK) {
		DebugMsg("CFreestyleApp", "Frame Buffer #2 could not be created.  HRESULT = %X", retVal);
		return S_FALSE;
	}

	return S_OK;
}

HRESULT CFreestyleApp::Initialize()
{

	#ifndef NDEBUG
		m_bRenderOverlays = true;
	#else if 
		if(IsDebugMode())
			m_bRenderOverlays = true;
		else
			m_bRenderOverlays = false;
	#endif

	if(SETTINGS::getInstance().getInstallerState() == 0) {
		
		// Create Render Targets for use with Predicated Tiling
		CreateRenderTargets();

		
		#define USING_PLUGIN
		// Initialize and load the plugin system
		#ifdef USING_PLUGIN
			PluginLoader::getInstance().Initialize();
		#endif
	
		//Start FTP Server
		if(hasInternetConnection())
			CFTPServer::getInstance();
	
		//Initialize and start the HTTP Downloader
		HTTPDownloader::getInstance();
		if(hasInternetConnection())
			HTTPDownloader::getInstance().Resume();
	
		// Sync time via NTP if settings are requesting it be updated at boot
		if (SETTINGS::getInstance().getNTPonStartup() > 0 && hasInternetConnection()) {
			NTP::getInstance().SyncTime();
		}

		// Set Fanspeed to desired user settings
		int nFanspeed = SETTINGS::getInstance().getFanSpeed();
		if (nFanspeed > MINIMUM_FANSPEED) {
			smc mySmc;
			mySmc.SetFanSpeed(1, nFanspeed);
		}

		if (SETTINGS::getInstance().getSambaServerOn() && hasInternetConnection()) {
			XNADDR addr;
			DWORD retval = XNetGetTitleXnAddr(&addr);

			DebugMsg("SambaServer", "XNetGetTitleXnAddr returned %d", retval);

			char ip[16];
			sprintf_s(ip, 16, "%d.%d.%d.%d", (byte)addr.ina.S_un.S_un_b.s_b1,
				(byte)addr.ina.S_un.S_un_b.s_b2,
				(byte)addr.ina.S_un.S_un_b.s_b3,
				(byte)addr.ina.S_un.S_un_b.s_b4
				);


			set_xbox_interface(ip, "255.255.255.0");  

			SambaServer::getInstance();
		}

		// Create the Overlay Font for Display
		m_OverlayFont.Create( "game:\\Media\\Fonts\\Arial_16.xpr" );
	
		// Confine text drawing to the title safe area
		m_OverlayFont.SetWindow( ATG::GetTitleSafeArea() );
	}
	
	// Initialize the XUI Class
	CFreestyleUIApp::getInstance().InitializeUI(NULL);

    return S_OK;
}

HRESULT CFreestyleApp::InitializeVersionInfo( void )
{
	// Initialize VersionManager and Print Current Version Information to the DebugLog
	VersionManager::getInstance();
	DebugMsg("FreestyleApp", "********************************************************************************");
	DebugMsg("FreestyleApp", "*********       Dash version: %s", VersionManager::getInstance().getFSDDashVersionAsString().c_str());
	DebugMsg("FreestyleApp", "*********       Skin version: %s", VersionManager::getInstance().getFSDSkinVersionAsString().c_str());
	DebugMsg("FreestyleApp", "*********       Plugin version: %s", VersionManager::getInstance().getFSDPluginVersionAsString().c_str());
	DebugMsg("FreestyleApp", "*********       Kernel version: %s", VersionManager::getInstance().getKernelAsString().c_str());
	DebugMsg("FreestyleApp", "********************************************************************************");
	
	// Enumerate the Current Versions for Display during the Overlay
	m_dwDashVer = VersionManager::getInstance().getFSDDashVersion();
	m_dwSkinVer = VersionManager::getInstance().getFSDSkinVersion();
	m_dwPluginVer = VersionManager::getInstance().getFSDPluginVersion();
	m_dwKernel = VersionManager::getInstance().getKernelVersion();

	// Set private variables to determine version overlay text
	m_bDashVer = !VersionManager::getInstance().IsVersionInfoNull(m_dwDashVer);
	m_bSkinVer = !VersionManager::getInstance().IsVersionInfoNull(m_dwSkinVer);
	m_bPluginVer = !VersionManager::getInstance().IsVersionInfoNull(m_dwPluginVer);

	// Return Successfully
	return S_OK;
}

HRESULT CFreestyleApp::Update()
{
    // Retrieve and dispatch input to the UI
    XINPUT_KEYSTROKE keyStroke;
    XInputGetKeystroke( XUSER_INDEX_ANY, XINPUT_FLAG_ANYDEVICE, &keyStroke );

	if(keyStroke.VirtualKey == VK_PAD_LTHUMB_PRESS) {
		DebugMsg("KeyPress", "Keystroke Flags = %x", keyStroke.Flags);
		DebugMsg("KeyPress", "Keystroke HidCode = %x", keyStroke.HidCode);
		DebugMsg("KeyPress", "Keystroke Unicode = %x", keyStroke.Unicode);
		DebugMsg("KeyPress", "Keystroke User Index = %x", keyStroke.UserIndex);
		DebugMsg("KeyPress", "Keystroke Virtual Key = %x", keyStroke.VirtualKey);

		XINPUT_CAPABILITIES caps;
		XInputGetCapabilities(keyStroke.UserIndex, keyStroke.Flags, &caps);

		DebugMsg("KeyPress", "Capabilities Flags = %x", caps.Flags );
		DebugMsg("KeyPress", "Capabilities Gamepad.bLeftTrigger = %x", caps.Gamepad.bLeftTrigger);
		DebugMsg("KeyPress", "Capabilities Gamepad.bRightTrigger = %x", caps.Gamepad.bRightTrigger );
		DebugMsg("KeyPress", "Capabilities Gamepad.sThumbLX = %x", caps.Gamepad.sThumbLX );
		DebugMsg("KeyPress", "Capabilities Gamepad.sThumbLY = %x", caps.Gamepad.sThumbLY );
		DebugMsg("KeyPress", "Capabilities Gamepad.sThumbRX = %x", caps.Gamepad.sThumbRX );
		DebugMsg("KeyPress", "Capabilities Gamepad.sThumbRY = %x", caps.Gamepad.sThumbRY );
		DebugMsg("KeyPress", "Capabilities Gamepad.wButtons = %x", caps.Gamepad.wButtons );
		DebugMsg("KeyPress", "Capabilities SubType = %x", caps.SubType );
		DebugMsg("KeyPress", "Capabilities Type = %x", caps.Type );
		DebugMsg("KeyPress", "Capabilities Vibration = %x", caps.Vibration );

		if(caps.SubType == XINPUT_DEVSUBTYPE_GAMEPAD)
			ScreenShot(m_pd3dDevice);
	} else {
		CFreestyleUIApp::getInstance().DispatchXuiInput( &keyStroke );
	}

	// Notify Observers that it's time to update
	notifyFrameUpdate();

	// Update the next Xui Frame
	CFreestyleUIApp::getInstance().UpdateUI(NULL);

    return S_OK;
}

HRESULT CFreestyleApp::Render()
{
	// Notify RenderLoopObservers that it is time to PreRender
	notifyFramePreRender();

	// Pre Render any XUI effects or shadow effects prior to rendering the scene
	CFreestyleUIApp::getInstance().PreRenderUI(NULL);

	// Render the entire screen to the backbuffer
	RenderScene();

	// Notify RenderLoopObservers that it is time to PostRender
	notifyFramePostRender();

	// Synchronize the buffer to the presentation interval to avoid screen tearings
	m_pd3dDevice->SynchronizeToPresentationInterval();
	
	// After the final scene is built, check to see if a screenshot request was flagged
	if(ScreenshotCommand::getScreenshotRequest()) {		
		ScreenshotCommand::TakeScreenshot();
	}

	// Present the back buffer and swap frame buffers to continue rendering
	m_pd3dDevice->Swap( m_pFrontBuffer[ m_dwCurFrontBuffer ], NULL );
	m_dwCurFrontBuffer = ( m_dwCurFrontBuffer + 1 ) % 2;

    return S_OK;
}

HRESULT CFreestyleApp::RenderScene()
{
	// Reset the CPU Processing Timer
	m_CPUTimer.Reset();

	m_pd3dDevice->SetRenderTarget( 0, m_pBackBuffer );
	m_pd3dDevice->SetDepthStencilSurface( m_pDepthBuffer );

	// Begin Rendering our Scene and all of it's components
	m_pd3dDevice->BeginTiling( 0, ARRAYSIZE(g_dwTiles), g_dwTiles, &g_vClearColor, 1, 0 );
	{

		// Notify RenderLoopObservers that it's time to Render Pre XUI
		notifyFrameRenderPreXUI();

		// Render XUI to the device
		CFreestyleUIApp::getInstance().RenderUI( m_pd3dDevice, SETTINGS::getInstance().getHorizOverscan(), SETTINGS::getInstance().getVertOverscan(), g_dwFrameWidth, g_dwFrameHeight );

		// Notify RenderLoopObservers that it's time to Render Post XUI
		notifyFrameRenderPostXUI();

		// Render Overlays
		if(m_bRenderOverlays)
			RenderOverlays();
	}
	m_pd3dDevice->EndTiling( 0, NULL, m_pFrontBuffer[m_dwCurFrontBuffer], NULL, 1, 0, NULL );

	// Return Successfully
    return S_OK;
}

HRESULT CFreestyleApp::RenderOverlays( void )
{
	WCHAR szText[100];
	
	// Mark current statistics for calculations
	m_fRenderTime = (FLOAT) m_CPUTimer.GetElapsedTime();
	m_FrameRateTimer.MarkFrame();
	m_dwFrameCounter++;
	
	// Store available ram for display
	if(m_dwFrameCounter > MEMORY_POLL_RATE) {
		MEMORYSTATUS memStat;
		GlobalMemoryStatus(&memStat);
		m_dwAvailableRam = memStat.dwAvailPhys;

		// Reset Frame Counter
		m_dwFrameCounter = 0;
	}

	// Begin Rendering Font
	m_OverlayFont.Begin();
	
	// Display Project Title In Upper Right
	if(m_bShowProjectMode) {
		m_OverlayFont.SetScaleFactors( 1.2f, 1.2f );
		m_OverlayFont.DrawText( 0, 0, OVERLAY_FONTCOLORA, OVERLAY_PROJECT_MODE );
	}

	// Display Frames Per Second
	if(m_bShowFrameRate) {
		m_OverlayFont.SetScaleFactors( 1.0f, 1.0f );
		m_OverlayFont.DrawText( 0, 0, OVERLAY_FONTCOLORB, m_FrameRateTimer.GetFrameRate(), ATGFONT_RIGHT );
	}

	// Display CPU Render Time (Time it takes to render 1 frame)
	if(m_bShowCPURender) {
		m_OverlayFont.SetScaleFactors( 1.0f, 1.0f );
		swprintf_s(szText, L"CPU Render: %0.4f ms", m_fRenderTime * 1000 );
		m_OverlayFont.DrawText( 0, 20, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );
	}

	// Display Free Memory
	if(m_bShowFreeRam) {
		m_OverlayFont.SetScaleFactors( 1.0f, 1.0f );
		swprintf_s(szText, L"Free Mem:  %d bytes", m_dwAvailableRam );
		m_OverlayFont.DrawText( 0, 40, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );
	}

	if(m_bShowVersionInfo) {
		// Display Version Information
		m_OverlayFont.SetScaleFactors( 0.75f, 0.75f );

		if(m_bDashVer) swprintf_s( szText, OVERLAY_VERSION_EXISTS, L"Dash", m_dwDashVer.Version.dwMajor, m_dwDashVer.Version.dwMinor, VersionManager::getInstance().ConvertTypeToLetterW(m_dwDashVer.Version.dwVersionType, false).c_str(), m_dwDashVer.Version.dwRevision);
		else swprintf_s( szText, OVERLAY_VERSION_MISSING, L"Dash" );
		m_OverlayFont.DrawText( 0, 60, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );

		if(m_bSkinVer) swprintf_s( szText, OVERLAY_VERSION_EXISTS, L"Skin", m_dwSkinVer.Version.dwMajor, m_dwSkinVer.Version.dwMinor, VersionManager::getInstance().ConvertTypeToLetterW(m_dwSkinVer.Version.dwVersionType, false).c_str(), m_dwSkinVer.Version.dwRevision);
		else swprintf_s( szText, OVERLAY_VERSION_MISSING, L"Skin");
		m_OverlayFont.DrawText( 0, 75, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );

		if(m_bPluginVer) swprintf_s( szText, OVERLAY_VERSION_EXISTS, L"Plugin", m_dwPluginVer.Version.dwMajor, m_dwPluginVer.Version.dwMinor, VersionManager::getInstance().ConvertTypeToLetterW(m_dwPluginVer.Version.dwVersionType, false).c_str(), m_dwPluginVer.Version.dwRevision);
		else swprintf_s( szText, OVERLAY_VERSION_MISSING, L"Plugin");
		m_OverlayFont.DrawText( 0, 90, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );
 
		swprintf_s( szText, L"Kernel Version:  %d.%d.%d.%d", m_dwKernel.Major, m_dwKernel.Minor, m_dwKernel.Build, m_dwKernel.QFE);
		m_OverlayFont.DrawText(0, 105, OVERLAY_FONTCOLORB, szText, ATGFONT_RIGHT );
	}

	m_OverlayFont.End();

	return S_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//    iRenderLoopObserver Functions and Implementation
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CFreestyleApp::AddObserver(iRenderLoopObserver& Observer) {
	// Add new observer to our observer list
	renderLoopObservers.insert(renderLoopItem::value_type(&Observer, &Observer));

	return S_OK;
}

HRESULT CFreestyleApp::RemoveObserver(iRenderLoopObserver& Observer) {
	// Remove observer from our observer list
	renderLoopObservers.erase(&Observer);

	return S_OK;
}

HRESULT CFreestyleApp::ClearObservers( void ) {
	// Clear all of the observers from our list
	renderLoopObservers.clear();

	return S_OK;
}

HRESULT CFreestyleApp::notifyFrameUpdate( void ) {
	// Let observers know that a message was triggered
	for(renderLoopItem::const_iterator it = renderLoopObservers.begin(); it != renderLoopObservers.end(); ++it )
		it->first->handleOnFrameUpdate();

	return S_OK;
}

HRESULT CFreestyleApp::notifyFramePreRender( void ) {
	// Let observers know that a message was triggered
	for(renderLoopItem::const_iterator it = renderLoopObservers.begin(); it != renderLoopObservers.end(); ++it )
		it->first->handleOnFramePreRender();

	return S_OK;
}

HRESULT CFreestyleApp::notifyFrameRenderPreXUI( void ) {
	// Let observers know that a message was triggered
	for(renderLoopItem::const_iterator it = renderLoopObservers.begin(); it != renderLoopObservers.end(); ++it )
		it->first->handleOnFrameRenderPreXUI();

	return S_OK;
}

HRESULT CFreestyleApp::notifyFrameRenderPostXUI( void ) {
	// Let observers know that a message was triggered
	for(renderLoopItem::const_iterator it = renderLoopObservers.begin(); it != renderLoopObservers.end(); ++it )
		it->first->handleOnFrameRenderPostXUI();

	return S_OK;
}

HRESULT CFreestyleApp::notifyFramePostRender( void ) {
	// Let observers know that a message was triggered
	for(renderLoopItem::const_iterator it = renderLoopObservers.begin(); it != renderLoopObservers.end(); ++it )
		it->first->handleOnFramePostRender();

	return S_OK;
}