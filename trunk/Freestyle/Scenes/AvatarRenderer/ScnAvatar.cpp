#pragma once
#include "stdafx.h"

#include "../../Application/FreestyleApp.h"
#include "../../Tools/Settings/Settings.h"
#include "ScnAvatar.h"

static const D3DRECT g_SmallResTiles[2] =
{
	{	0,		0,		512,		256 },
	{	0,	  256,		512,		512 }
};

static const D3DRECT g_LargeResTiles[4] = 
{
	{	0,		0,		1024,		256 },
	{	0,	  256,		1024,		512 },
	{	0,	  512,		1024,		768 },
	{	0,	  768,		1024,		1024}
};


CScnAvatar::CScnAvatar()
{
	// First let's properly initialize our direct 3d components
	m_pRenderTarget = NULL;
	m_pDepthStencil = NULL;
	m_pTexture = NULL;

	bStateChanged = false;
}

CScnAvatar::~CScnAvatar()
{
	// Remove ourself from any observer classes
	SignInMonitor::getInstance().RemoveObserver(*this);
	CFreestyleApp::getInstance().RemoveObserver(*this);

	// Make sure it's not at a vital part in the rendering loop
	bIsRendered = false;
	while(!bSafeToFree) {
		Sleep(0);
	}

	// Free our avatar resources
	FreeAvatarResources();

	// Release our Direct3D Resources
	SAFE_RELEASE( m_pTexture );
	SAFE_RELEASE( m_pRenderTarget );
	SAFE_RELEASE( m_pDepthStencil );

	if( m_hAvatarBrush != NULL ) {
		XuiDestroyBrush(m_hAvatarBrush);
		m_hAvatarBrush = NULL;
	}
}

HRESULT CScnAvatar::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{

	bSafeToFree = false;

	// Read in scene settings from skin.xml
	InitializeChildren();

	// Read in skin.xml settings
	ReadSettings();

	// Initialize Render Targets and Load Avatar MetaData
	InitializeAvatarScene();

	// Add Class to RenderLoop Injector List
	CFreestyleApp::getInstance().AddObserver(*this);
	SignInMonitor::getInstance().AddObserver(*this);

	bSafeToFree = true;

	return S_OK;
}

HRESULT CScnAvatar::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	if(pGetSourceImageData->iData == 0){
		if(!m_pTexture || !bIsRendered || !CheckUserSettings())
			return S_OK;

		pGetSourceImageData->hBrush = m_hAvatarBrush;
		bHandled = TRUE;
	}

	return S_OK;
}

void CScnAvatar::InitializeChildren( void )
{
	// Initialize our controls and fill in our states
	XuiInitControl(L"AvatarPresenter", m_AvatarPresenter, &hasAvatarPresenter);
	XuiInitControl(L"AvatarShadow", m_AvatarShadow, &hasAvatarShadow);
	XuiInitControl(L"AvatarSilhouette", m_AvatarSilhouette, &hasAvatarSilhouette);
	XuiInitControl(L"Rotation", m_RotationSlider, &hasRotationSlider);
	XuiInitControl(L"AutoRotate", m_AutoRotate, &hasAutoRotate);

	if(hasRotationSlider) {
		int nValue = 0, nDegrees = 0;
		m_RotationSlider.GetValue(&nValue);
		nDegrees = nValue % 360;
		fAngle = (nDegrees * XM_PI) / 180.0f;
	}

	if(hasAutoRotate)
		m_AutoRotate.GetRange(&nAutoRotateMin, &nAutoRotateMax);
}

void CScnAvatar::ReadSettings( void )
{
	LoadSettings("ScnAvatar", *this);
	
	string strBuffer = "";

	strBuffer = GetSetting("INTERNALRES", "LARGE");
	if(strcmp(make_lowercaseA(strBuffer).c_str(), "small")==0)
		bRenderHighRes = false;
	else
		bRenderHighRes = true;

	strBuffer = GetSetting("PLAYWALKINANIM", "FALSE");
	if(strcmp(make_lowercaseA(strBuffer).c_str(), "false")==0)
		bPlayWalkInAnim = false;
	else
		bPlayWalkInAnim = true;

	strBuffer = GetSetting("PLAYERINDEX", "1");
	dwPlayerId = atoi(strBuffer.c_str());

	strBuffer = GetSetting("HIDEONERROR", "FALSE");
	if(strcmp(make_lowercaseA(strBuffer).c_str(), "true")==0)
		bHideSceneOnError = true;
	else
		bHideSceneOnError = false;

	strBuffer = GetSetting("AVATARSHADOW", "BOTH");
	if(strcmp(make_lowercaseA(strBuffer).c_str(), "missingonly")==0)
		dwShadowState = 0;
	else if(strcmp(make_lowercaseA(strBuffer).c_str(), "avataronly")==0)
		dwShadowState = 1;
	else
		dwShadowState = 2;

	strBuffer = GetSetting("AUTOROTATE", "OFF");
	if(strcmp(make_lowercaseA(strBuffer).c_str(), "on")==0)
		bAutoRotate = true;
	else
		bAutoRotate = false;

	strBuffer = GetSetting("ANIMATIONINDEX", "0");
	if(strcmp(make_lowercaseA(strBuffer).c_str(), "0")==0)
		dwAnimationIndex = 0;
	else
		dwAnimationIndex = atoi(strBuffer.c_str());

	strBuffer = GetSetting("ROTATION", "0"); // Rotation Angle in Degrees
	fAngle = (atoi(strBuffer.c_str()) * XM_PI) / 180.0f;
	
	dwPlayerId = dwPlayerId - 1;

	if(dwAnimationIndex > ANIMATION_COUNT - 1)
		dwAnimationIndex = 0;

}

void CScnAvatar::InitializeAvatarScene( void )
{	
	// Set some initial variables required for initialization
	IDirect3DDevice9 * pD3DDevice = CFreestyleApp::getInstance().m_pd3dDevice;
	m_vAvatarData.s_bAnimationsLoaded = FALSE;
	
	// Initialize the Avatar library
    static const DWORD dwAssetLoadHardwareThread = 4;
    static const DWORD dwJointsBufferCount = 2;
    XAvatarInitialize(  XAVATAR_COORDINATE_SYSTEM_RIGHT_HANDED, 
                        XAVATAR_INITIALIZE_FLAGS_ENABLERENDERER, 
                        dwAssetLoadHardwareThread,
                        dwJointsBufferCount,
						pD3DDevice);

	// Load the avatar data for this instance
	LoadLocalAvatar();

	// Initialize view parameters
    XVIDEO_MODE VideoMode;
    XGetVideoMode( &VideoMode );
    FLOAT fAspectRatio = VideoMode.fIsWideScreen == TRUE ? (16.0f / 9.0f) : (4.0f / 3.0f);

    m_matProj    = XMMatrixPerspectiveFovRH( XM_PI/4, fAspectRatio, 0.01f, 20.0f );    
    m_vEyePt     = XMVectorSet( 0.0f, 0.75f, 1.1f, 0.0f );
    m_vUp        = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMMATRIX lookAtMatrix   = XMMatrixRotationRollPitchYaw( 0.0f, 0.0f, 0.0f );
	XMVECTOR m_vLookToZ     = XMVector3Transform(XMVectorSet( 0.0f, 0.0f, -1.0f, 1.0f ), lookAtMatrix);
	m_matView               = XMMatrixLookToRH( m_vEyePt, m_vLookToZ, m_vUp );

	// Initialize RenderTargets and Texture Properties
	DWORD dwTileHeight = 256;
	DWORD dwTileWidth, dwTextureSize;

	if(bRenderHighRes == TRUE){
		dwTextureSize = 1024;
		dwTileWidth = 1024;
	}else{
		dwTextureSize = 512;
		dwTileWidth = 512;
	}
	
    D3DSURFACE_PARAMETERS params = {0};

    pD3DDevice->CreateRenderTarget(
        dwTileWidth, dwTileHeight, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pRenderTarget, &params );

    params.Base = m_pRenderTarget->Size / GPU_EDRAM_TILE_SIZE;
    params.HierarchicalZBase = 0;

    pD3DDevice->CreateDepthStencilSurface(
        dwTileWidth, dwTileHeight, D3DFMT_D24S8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pDepthStencil, &params );

	pD3DDevice->CreateTexture( dwTextureSize, dwTextureSize, 1, 0,D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);

	XuiAttachTextureBrush( m_pTexture, &m_hAvatarBrush );
}

void CScnAvatar::LoadLocalAvatar( void )
{
	HRESULT retVal = NULL;
	bool bUseRandomAvatar = false;

	// If any resources are loaded, free before loading new resources
	FreeAvatarResources();

	// Ensure that that player id is valid, if not, make it random
	if(dwPlayerId > 3 || dwPlayerId < 0)
		bUseRandomAvatar = true;
		// Scale Identity Matrix 
	const XMMATRIX m_matScale = XMMatrixScaling( 1.50f, 0.80f, 1.00f );
	const XMMATRIX m_matIdent = XMMatrixIdentity();
	const XMMATRIX m_matWorld = XMMatrixMultiply(m_matScale, m_matIdent);
	const XMMATRIX m_matTrans = XMMatrixTranslation(0, 0, -2.85f);

	// Create Rotational Matrix
	const XMMATRIX m_matRotate = XMMatrixRotationY(fAngle);
	const XMMATRIX m_matMult1 = XMMatrixMultiply(m_matRotate, m_matWorld);
	
	m_vAvatarData.m_matWorld = XMMatrixMultiply(m_matMult1, m_matTrans);
	m_vAvatarData.m_pRenderer = NULL;

	// Load custom avatar assets here
	if(!bUseRandomAvatar) {
		XUserGetXUID( dwPlayerId, &m_vAvatarData.m_PlayerXuid);
		DWORD ret = XAvatarGetMetadataLocalUser(dwPlayerId, &m_vAvatarData.m_Metadata, NULL);
		if(ERROR_SUCCESS != ret) {
			bIsRendered = false;
			ShowMissingAvatarElement();
			return;
		}
	} else {
		XUserGetXUID( 0, &m_vAvatarData.m_PlayerXuid);
		XAvatarGetMetadataRandom(XAVATAR_BODY_TYPE_ALL, 1, &m_vAvatarData.m_Metadata, NULL);
	}

	// Create the avatar renderer
	XAvatarCreateRenderer(	&m_vAvatarData.m_Metadata,
							XAVATAR_COMPONENT_MASK_ALL,
                            XAVATAR_SHADOW_SIZE_SMALL,
                            XAVATAR_MIPMAP_RECOMMENDED_BUFFER_SIZE,
                            0,
                            &m_vAvatarData.m_pRenderer );

	if(m_vAvatarData.m_pRenderer == NULL) {
		bIsRendered = false;
		return;
	}

	// Set Avatar Level of Detail to Low	
	m_vAvatarData.m_pRenderer->SetLevelOfDetail(XAVATAR_RENDER_LOD_LOW);

	// Load animations and proceed to rendering
	LoadAvatarAnimations();
	
	HideMissingAvatarElement();
	bIsRendered = true;
}

void CScnAvatar::ShowMissingAvatarElement( void )
{
	if(bHideSceneOnError)
		this->SetShow(false);

	if(hasAvatarSilhouette)
		m_AvatarSilhouette.SetShow(true);

	if(hasAvatarShadow)
	{
		if(dwShadowState == 2 || dwShadowState == 0)
			m_AvatarShadow.SetShow(true);
		else
			m_AvatarShadow.SetShow(false);
	}

	DebugMsg("AvatarRenderer", "Player is not signed in or does not have an avatar created");
}

void CScnAvatar::HideMissingAvatarElement( void )
{
	if(hasAvatarSilhouette)
		m_AvatarSilhouette.SetShow(false);

	if(hasAvatarShadow)
	{
		if(dwShadowState == 2 || dwShadowState == 1)
			m_AvatarShadow.SetShow(true);
		else
			m_AvatarShadow.SetShow(false);
	}

	DebugMsg("AvatarRenderer", "Player is signed Out and Avatar is Present");

}

bool CScnAvatar::CheckUserSettings( void )
{
	bool m_SettingValue;
	bool m_SceneState = this->IsShown() == 0 ? false : true;

	m_SettingValue = SETTINGS::getInstance().getShowAvatar() == 0 ? false : true;

	if(m_SceneState != m_SettingValue)
		this->SetShow(m_SettingValue);

	return m_SettingValue;
}

void CScnAvatar::UpdateAvatar( void )
{
	if(!CheckUserSettings())
		return;

	if(bStateChanged)
	{
		if(bPlayerOnline) UserSignedIn();
		else UserSignedOut();
		bStateChanged = false;
	}

	bSafeToFree = false;
	
	// If something didn't initialize right or ran out of memory, don't render
	if( !m_pTexture || !m_pRenderTarget || !m_pDepthStencil || !bIsRendered)
		return;

	FLOAT fDeltaTime = ( FLOAT )m_Timer.GetElapsedTime();

	if(hasAutoRotate && bAutoRotate)
	{
		
		int nValue = 0;
		m_AutoRotate.GetValue(&nValue);

		int nDegrees = nValue % 360;
		fAngle = (nDegrees * XM_PI) / 180.0f;
		
		// Scale Identity Matrix 
		const XMMATRIX m_matScale = XMMatrixScaling( 1.50f, 0.80f, 1.00f );
		const XMMATRIX m_matIdent = XMMatrixIdentity();
		const XMMATRIX m_matWorld = XMMatrixMultiply(m_matScale, m_matIdent);
		const XMMATRIX m_matTrans = XMMatrixTranslation(0, 0, -2.85f);

		// Create Rotational Matrix
		const XMMATRIX m_matRotate = XMMatrixRotationY(fAngle);
		const XMMATRIX m_matMult1 = XMMatrixMultiply(m_matRotate, m_matWorld);
		
		m_vAvatarData.m_matWorld = XMMatrixMultiply(m_matMult1, m_matTrans);
	}

	m_vAvatarData.m_pRenderer->Update(fDeltaTime);
	m_vAvatarData.m_pRenderer->RenderShadow(CFreestyleApp::getInstance().m_pd3dDevice, m_vAvatarData.m_matWorld, m_matView, m_matProj);
}

void CScnAvatar::RenderAvatar( void )
{
	// If something didn't initialize right or ran out of memory, don't render
	if( !m_pTexture || !m_pRenderTarget || !m_pDepthStencil || !bIsRendered)
		return;

	// Define devices and surfaces for use
	IDirect3DDevice9  * pD3DDevice = CFreestyleApp::getInstance().m_pd3dDevice;
	IDirect3DSurface9 * pOriginalRenderTarget = NULL;
	IDirect3DSurface9 * pOriginalDepthStencil = NULL;

	// Get temporary Render and Stencil surfaces to recall later
	pD3DDevice->GetRenderTarget( 0, &pOriginalRenderTarget );
	pD3DDevice->GetDepthStencilSurface( &pOriginalDepthStencil);

	// Set new Render Target and Stencil for Rendering
	const D3DVECTOR4 clearColor = { 0.75f, 0.78f, 0.86f, 0.0f };
	pD3DDevice->SetRenderTarget( 0, m_pRenderTarget );
	pD3DDevice->SetDepthStencilSurface( m_pDepthStencil );

	// Enable all color Channels
	pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );
	
	// Render the avatar using Predicated Tiling
	if(bRenderHighRes)
		pD3DDevice->BeginTiling( 0, ARRAYSIZE(g_LargeResTiles), g_LargeResTiles, &clearColor, 1, 0 );
	else
		pD3DDevice->BeginTiling( 0, ARRAYSIZE(g_SmallResTiles), g_SmallResTiles, &clearColor, 1, 0 );

	m_vAvatarData.m_pRenderer->Render(pD3DDevice, m_vAvatarData.m_matWorld, m_matView, m_matProj);
	pD3DDevice->EndTiling( 0, NULL, m_pTexture, NULL, 1, 0, NULL );    	

	// Restore original RenderTargets and Stencils
	pD3DDevice->SetRenderTarget( 0, pOriginalRenderTarget );
	pD3DDevice->SetDepthStencilSurface(pOriginalDepthStencil);

}

void CScnAvatar::LoadAvatarAnimations()
{
	// Load Avatar Animations here
}

void CScnAvatar::FreeAvatarResources( void )
{	
	if(m_vAvatarData.s_bAnimationsLoaded)
    {
        for( DWORD i = 0; i < ANIMATION_COUNT; i++ )
        {
            if( m_vAvatarData.s_pAnimations[i] )
            {
                m_vAvatarData.s_pAnimations[i]->Release();
            }
        }

        m_vAvatarData.s_bAnimationsLoaded = FALSE;
    }

    if( m_vAvatarData.m_pRenderer )
    {
		if(bSafeToFree)
			m_vAvatarData.m_pRenderer->Release();
        m_vAvatarData.m_pRenderer = NULL;
	}

	bIsRendered = false;

}