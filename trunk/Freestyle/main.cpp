//----------------------------------------------------------------------------------
// XuiTabbedScene.cpp
//
// An example of a XUI tabbed scene.
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------
#include "stdafx.h"

#include "./Tools/Managers/Drives/DrivesManager.h"
#include "./Tools/Settings/Settings.h"
#include "./Application/FreestyleApp.h"

//----------------------------------------------------------------------------------
// Name: main
// Desc: Application entry point.
//----------------------------------------------------------------------------------
VOID __cdecl main()
{ 
#ifdef _DEBUG
	// This will dump memory leaks
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	DWORD dwLaunchDataSize = 0;    
    DWORD dwStatus = XGetLaunchDataSize( &dwLaunchDataSize );
    if( dwStatus == ERROR_SUCCESS )
    {
        BYTE* pLaunchData = new BYTE [ dwLaunchDataSize ];
        dwStatus = XGetLaunchData( pLaunchData, dwLaunchDataSize );
    }

	SetUnhandledExceptionFilter(UnHandleExceptionFilter);
	
	ATG::GetVideoSettings( &(CFreestyleApp::getInstance().m_d3dpp.BackBufferWidth), &(CFreestyleApp::getInstance().m_d3dpp.BackBufferHeight) );

	CFreestyleApp::getInstance().m_d3dpp.BackBufferCount        = 1;
    CFreestyleApp::getInstance().m_d3dpp.MultiSampleType        = D3DMULTISAMPLE_4_SAMPLES;
    CFreestyleApp::getInstance().m_d3dpp.EnableAutoDepthStencil = FALSE;
    CFreestyleApp::getInstance().m_d3dpp.DisableAutoBackBuffer  = TRUE;
	CFreestyleApp::getInstance().m_d3dpp.DisableAutoFrontBuffer = TRUE;
    CFreestyleApp::getInstance().m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	CFreestyleApp::getInstance().m_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;

	CFreestyleApp::getInstance().m_dwDeviceCreationFlags      |= D3DCREATE_BUFFER_2_FRAMES | D3DCREATE_CREATE_THREAD_ON_0;

	//moved some initialisations here to have settingsread before letterbox flag is set:

	//Init Network
	CFreestyleApp::getInstance().setInternetConnection(InitNetwork());
	//Start Debugger
	if(CFreestyleApp::getInstance().hasInternetConnection())
		StartDebugger(3000,3001);
	
	//Data Path :
	if (FileExists("game:\\Data\\Databases\\fsd2data.db"))
	{
		SETTINGS::getInstance().setDataPath("game:\\Data\\");
		SETTINGS::getInstance().setDataPathX("file://game:/Data/");
	}
	else
	{
		// This is our first run, so let's set our first run flag
		//   This can be used to pop up welcome messages at loadtime

		CFreestyleApp::getInstance().setFirstRun(true);
	}

	DebugMsg("main", "FirstRun(%s)", CFreestyleApp::getInstance().getFirstRun() ? "true" : "false");

	//Mount Drives
	DrivesManager::getInstance();

	XVIDEO_MODE VideoMode; 
	XMemSet( &VideoMode, 0, sizeof(XVIDEO_MODE) ); 
	XGetVideoMode( &VideoMode );

	
	if (!VideoMode.fIsWideScreen && (SETTINGS::getInstance().getEnableLetterbox() == 0))
	{
	        CFreestyleApp::getInstance().m_d3dpp.Flags |=  D3DPRESENTFLAG_NO_LETTERBOX;
	}
	
	// Run the scene.  
	CFreestyleApp::getInstance().Run();
}