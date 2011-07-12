#pragma once

#include "FreestyleUIApp.h"
#include "../Tools/Managers/Version/VersionManager.h"

#ifdef _DEBUG
	#define _PROFILE_BUILD
#endif 

#define MINIMUM_FANSPEED			45

#define OVERLAY_FONTCOLORA			0xFFFFFF00
#define OVERLAY_FONTCOLORB			0xFFFFFF00
#define OVERLAY_VERSION_EXISTS		L"%s Version:  %d.%d%s Rev%d"
#define OVERLAY_VERSION_MISSING		L"%s Version:  Missing"

#ifdef _DEBUG
	#define OVERLAY_PROJECT_MODE	L"*** DEBUG BUILD ***"
#else if
	#define OVERLAY_PROJECT_MODE	L"*** RELEASE BUILD ***"
#endif

#ifndef _PROFILE_BUILD
#define MEMORY_POLL_RATE	240
#else if
#define MEMORY_POLL_RATE 1
#endif

class iRenderLoopObserver {
public:
	virtual void handleOnFrameUpdate(){};
	virtual void handleOnFramePreRender(){};
	virtual void handleOnFrameRenderPreXUI(){};
	virtual void handleOnFrameRenderPostXUI(){};
	virtual void handleOnFramePostRender(){};
};

HRESULT XuiTextureLoader(IXuiDevice *pDevice, LPCWSTR szFileName, XUIImageInfo *pImageInfo, IDirect3DTexture9 **ppTex);

static const D3DVECTOR4 g_vClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
static const DWORD g_dwTileWidth   = 1280;
static const DWORD g_dwTileHeight  = 256;
static const DWORD g_dwFrameWidth  = 1280;
static const DWORD g_dwFrameHeight = 720;
static const D3DRECT g_dwTiles[3] = 
{
    {             0,              0,  g_dwTileWidth,  g_dwTileHeight },
    {             0, g_dwTileHeight,  g_dwTileWidth, g_dwTileHeight * 2 },
    {             0, g_dwTileHeight * 2,  g_dwTileWidth, g_dwFrameHeight },
};

class CFreestyleApp : public ATG::Application
{
public:
	static CFreestyleApp& getInstance()
	{
		static CFreestyleApp singleton;
		return singleton;
	}

	HRESULT InitializeVersionInfo( void );

	// Registration to the RenderLoop Observer
	HRESULT AddObserver(iRenderLoopObserver& ref);
	HRESULT RemoveObserver(iRenderLoopObserver& ref);
	HRESULT ClearObservers( void );

	// iRenderLoopObserver Notifications
	HRESULT notifyFrameUpdate( void );
	HRESULT notifyFramePreRender( void );
	HRESULT notifyFrameRenderPreXUI( void );
	HRESULT notifyFrameRenderPostXUI( void );
	HRESULT notifyFramePostRender( void );
	
	// Setters for Overlay Variables
	bool getDisplayOverlays( void ) { return m_bRenderOverlays; }
	bool getDisplayFrameRate( void ) { return m_bShowFrameRate; }
	bool getDisplayProjectMode( void ) { return m_bShowProjectMode; }
	bool getDisplayRenderTime( void ) { return m_bShowCPURender; }
	bool getDisplayFreeMemory( void ) { return m_bShowFreeRam; }
	bool getDisplayVersionInfo( void ) { return m_bShowVersionInfo; }

	// Setters for Overlay Variables
	void setDisplayOverlays( bool bRenderOverlays ) { m_bRenderOverlays = bRenderOverlays; }
	void setDisplayFrameRate( bool bShowFrameRate ) { m_bShowFrameRate = bShowFrameRate; }
	void setDisplayProjectMode( bool bShowProjectMode ) { m_bShowProjectMode = bShowProjectMode; }
	void setDisplayRenderTime( bool bShowCPURender ) { m_bShowCPURender = bShowCPURender; }
	void setDisplayFreeMemory( bool bShowFreeRam ) { m_bShowFreeRam = bShowFreeRam; }
	void setDisplayVersionInfo( bool bShowVersionInfo ) { m_bShowVersionInfo = bShowVersionInfo; }

	bool hasInternetConnection( void ) { return m_bInternetConnection; }
	void setInternetConnection( bool bValue ) { m_bInternetConnection = bValue; }

	bool getFirstRun(void) { return m_bFirstRun; }
	void setFirstRun(bool bValue ) { m_bFirstRun = bValue; }

	ATG::Font m_OverlayFont;
	
protected:
	CFreestyleApp();								// Private constructor
	~CFreestyleApp() {}								// Private destructor
	CFreestyleApp(const CFreestyleApp&);			// Prevent copy-construction
	CFreestyleApp& operator=(const CFreestyleApp&); // Prevent assignment

	// Rendering surfaces and textures
    D3DSurface*                 m_pBackBuffer;
    D3DSurface*                 m_pDepthBuffer;
    D3DTexture*                 m_pFrontBuffer[2];
	D3DTexture*					m_pFinalFrontBuffer;
    DWORD                       m_dwCurFrontBuffer;
	
	// ATG Application interface
	HRESULT Update();
	HRESULT Initialize();
	HRESULT Render();

private:
	// ATG Functionality for font and timer
	ATG::Timer m_FrameRateTimer;
	ATG::Timer m_CPUTimer;

	// VersionInfo for overlay display
	bool m_bDashVer, m_bSkinVer, m_bPluginVer;
	VERSION_INFO m_dwDashVer, m_dwSkinVer, m_dwPluginVer;
	VERSION_KERNEL m_dwKernel;

	// Variable to hold MemoryStatus
	DWORD m_dwAvailableRam;

	bool bUpdaterMode;
	bool m_bFirstRun;

	// Overlay Variables
	bool m_bRenderOverlays;
	bool m_bShowFrameRate;
	bool m_bShowProjectMode;
	bool m_bShowCPURender;
	bool m_bShowFreeRam;
	bool m_bShowVersionInfo;
	float m_fRenderTime;
	DWORD m_dwFrameCounter;

	// Rendering functions
	HRESULT RenderScene();
	HRESULT CreateRenderTargets( void );
	HRESULT RenderOverlays( void );

	// Project has Internet
	bool m_bInternetConnection;

	// Map to hold a list of all the registerd observers
	std::map<iRenderLoopObserver* const, iRenderLoopObserver* const> renderLoopObservers;
	typedef std::map<iRenderLoopObserver* const, iRenderLoopObserver* const> renderLoopItem;

};