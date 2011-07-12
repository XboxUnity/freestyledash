#include "stdafx.h"

#include "../../Application/FreestyleApp.h"
#include "../../Tools/XFSD/XFSD.h"
#include "../../Tools/HTTP/HttpItem.h"
#include "../../Tools/HTTP/HTTPDownloader.h"

#include "ScnTeamFSD.h"


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

CScnTeamFSD::CScnTeamFSD()
{
	m_pRenderTarget = NULL;
	m_pDepthStencil = NULL;
	m_pTexture = NULL;
}

CScnTeamFSD::~CScnTeamFSD()
{
	bIsRendered = false;

	while(!bSafeToFree){
		Sleep(0);
	}

	FreeAvatarResources();

    if( m_pTexture )
        m_pTexture->Release();
    m_pTexture = NULL;
	
	if( m_pRenderTarget )
		m_pRenderTarget->Release();
	m_pRenderTarget = NULL;

	if( m_pDepthStencil )
		m_pDepthStencil->Release();
	m_pDepthStencil = NULL;

	if( m_hBrush != NULL )
		XuiDestroyBrush(m_hBrush);

	m_hBrush = NULL;

	CFreestyleApp::getInstance().RemoveObserver(*this);
	
}

HRESULT CScnTeamFSD::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{

	bSafeToFree = false;

	// Read in scene settings from skin.xml
	InitializeChildren();

	if(hasMemberIndex) 
		m_nMemberIndex = atoi(wstrtostr(m_MemberIndex.GetText()).c_str());
	else
		m_nMemberIndex = -1;

	// Initialize Render Targets and Load Avatar MetaData
	InitializeAvatarScene();

	// Add Class to RenderLoop Injector List
	CFreestyleApp::getInstance().AddObserver(*this);

	bSafeToFree = true;

	return S_OK;
}

HRESULT CScnTeamFSD::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	if(pGetSourceImageData->iData == 0){
		if(!m_pTexture || !bIsRendered)
			return S_OK;

		pGetSourceImageData->hBrush = m_hBrush;
		bHandled = TRUE;
	}

	return S_OK;
}

void CScnTeamFSD::InitializeChildren( void )
{
	HRESULT hr = NULL;

	hr = GetChildById(L"MemberIndex", &m_MemberIndex);
	hasMemberIndex = hr == S_OK;

	hr = GetChildById(L"MemberName", &m_MemberName);
	hasMemberName = hr == S_OK;
}

void CScnTeamFSD::InitializeAvatarScene( void )
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
	DWORD dwTileHeight = TILE_DIM_Y;
	DWORD dwTileWidth = TILE_DIM_X;
	DWORD dwTextureSize = TEXTURE_RES_Y;
	
    D3DSURFACE_PARAMETERS params = {0};

    pD3DDevice->CreateRenderTarget(
        dwTileWidth, dwTileHeight, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pRenderTarget, &params );

    params.Base = m_pRenderTarget->Size / GPU_EDRAM_TILE_SIZE;
    params.HierarchicalZBase = 0;

    pD3DDevice->CreateDepthStencilSurface(
        dwTileWidth, dwTileHeight, D3DFMT_D24S8, D3DMULTISAMPLE_4_SAMPLES, 0, 0, &m_pDepthStencil, &params );

	pD3DDevice->CreateTexture( dwTextureSize, dwTextureSize, 1, 0,D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);

	XuiAttachTextureBrush( m_pTexture, &m_hBrush );
}

void CScnTeamFSD::LoadLocalAvatar( void )
{
	HRESULT retVal = NULL;

	// If any resources are loaded, free before loading new resources
	FreeAvatarResources();
	
	// Load TeamFSD data from resource file
	XFSD xfsdFile;
	retVal = xfsdFile.OpenXFSD("TeamFSD", true);
	if(retVal != S_OK){
		DebugMsg("ScnTeamFSD", "Failed to open TeamFSD resource from XEX");
		bIsRendered = false;
		return;
	}

	// Extract the total member count for range checking
	int nMemberCount;
	xfsdFile.GetMemberCount(&nMemberCount);
	
	// If for some reason the member index is out of range, do not load
	if(m_nMemberIndex < 0 || m_nMemberIndex > nMemberCount - 1)
	{
		// Clean up and close out
		DebugMsg("ScnTeamFSD", "Member Index is out of range");
		xfsdFile.CloseXFSD();	
		bIsRendered = false;
		return;
	}

	// Set the scenes member name extracted from the file
	if(hasMemberName){
		m_MemberName.SetText(strtowstr(xfsdFile.GetMemberName(m_nMemberIndex)).c_str());
	}
	  
	// Scale Identity Matrix to make sure avatar looks correct on screen
	const XMMATRIX m_matScale = XMMatrixScaling( 1.50f, 0.80f, 1.00f );
	const XMMATRIX m_matIdent = XMMatrixIdentity();
	const XMMATRIX m_matWorld = XMMatrixMultiply(m_matScale, m_matIdent);
	const XMMATRIX m_matTrans = XMMatrixTranslation(0, 0.05f, -0.85f);

	// Create Rotational Matrix wtih any preset rotational values
	const XMMATRIX m_matRotate = XMMatrixRotationY(0);
	const XMMATRIX m_matMult1 = XMMatrixMultiply(m_matRotate, m_matWorld);
	
	// Multiply the Rotational Matrix with the translation matrix to get world matrix
	m_vAvatarData.m_matWorld = XMMatrixMultiply(m_matMult1, m_matTrans);
	m_vAvatarData.m_pRenderer = NULL;

	// Extract Member Avatar Metadata from XEX Resource
	retVal = xfsdFile.GetMemberMetadata(m_nMemberIndex, &m_vAvatarData.m_metadata);
	if(retVal != S_OK)
	{
		DebugMsg("ScnTeamFSD", "Failed to extract valid metadata for Member ID: %d", m_nMemberIndex);
		bIsRendered = false;
		xfsdFile.CloseXFSD();
		return; 
	}

	// Done with the XEX Resource so we can close it
	xfsdFile.CloseXFSD();

	// Load custom avatar assets here
	LoadCustomAvatarAsset();

	// Create the avatar renderer
	XAvatarCreateRenderer(	&m_vAvatarData.m_metadata,
							XAVATAR_COMPONENT_MASK_ALL,
                            XAVATAR_SHADOW_SIZE_SMALL,
                            XAVATAR_MIPMAP_RECOMMENDED_BUFFER_SIZE,
                            0,
                            &m_vAvatarData.m_pRenderer );

	if(m_vAvatarData.m_pRenderer == NULL) {
		bIsRendered = false;
		XNotifyQueueUICustom(L" Please install the Avatar Update to get the full experience ");
		return;
	}

	// Set Avatar Level of Detail to Low	
	m_vAvatarData.m_pRenderer->SetLevelOfDetail(XAVATAR_RENDER_LOD_LOW);

	// Load animations and proceed to rendering
	LoadAvatarAnimations();
	bIsRendered = true;
}

void CScnTeamFSD::LoadAvatarAnimations( void )
{
	// Need to do still
	//LoadAnimationFromFile( "game:\\Skins\\Default\\Walking.bin", &m_vAvatarData.s_pAnimations[0] );
	//XAvatarLoadAnimation( &XAVATAR_ANIMATION_GENERIC_WAVE, 0, &m_vAvatarData.s_pAnimations[1] );
//	LoadAnimationFromFile( "game:\\Skins\\Default\\Matrix.bin", &m_vAvatarData.s_pAnimations[2] );
	//LoadAnimationFromFile( "game:\\Skins\\Default\\Hover_Anim.bin", &m_vAvatarData.s_pAnimations[3] );
	//if(IsUserSignedIn(1))
	//	m_vAvatarData.m_pRenderer->PlayAnimations(2, m_vAvatarData.s_pAnimations, XAVATAR_PLAYANIMATIONS_FLAGS_DEFAULT);
	//m_vAvatarData.s_bAnimationsLoaded = TRUE;
}

void CScnTeamFSD::UpdateAvatar( void )
{
	bSafeToFree = false;
	
	// If something didn't initialize right or ran out of memory, don't render
	if( !m_pTexture || !m_pRenderTarget || !m_pDepthStencil || !bIsRendered)
		return;

	FLOAT fDeltaTime = ( FLOAT )m_Timer.GetElapsedTime();

	m_vAvatarData.m_pRenderer->Update(fDeltaTime);
	m_vAvatarData.m_pRenderer->RenderShadow(CFreestyleApp::getInstance().m_pd3dDevice, m_vAvatarData.m_matWorld, m_matView, m_matProj);
}

HRESULT CScnTeamFSD::LoadCustomAvatarAsset()
{
 /*   BYTE* pAvatarAssetBuffer    = NULL  ;
    DWORD dwAvatarAssetSize     =   0;
    CHAR strFileName[]="game:\\Skins\\Default\\avatar_tshirt.bin";  

    // Load file to pAvatarAssetBuffer
    {

        HANDLE hFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                       FILE_FLAG_SEQUENTIAL_SCAN, NULL );
        if( hFile == INVALID_HANDLE_VALUE )
        {
            DebugMsg( "AvatarRenderer", "Could not find  file." );
            return E_FAIL;
        }
        dwAvatarAssetSize = GetFileSize( hFile, NULL );
        pAvatarAssetBuffer = reinterpret_cast<BYTE*>( XMemAlloc( dwAvatarAssetSize,                                                           
                                                      MAKE_XALLOC_ATTRIBUTES( 0,
                                                                              TRUE,
                                                                              TRUE,
                                                                              FALSE,
                                                                              SAMPLE_ALLOCATOR_ID,
                                                                              XALLOC_ALIGNMENT_16,
                                                                              XALLOC_MEMPROTECT_READWRITE,
                                                                              FALSE,
                                                                              XALLOC_MEMTYPE_HEAP)));
        //assert( dwAvatarAssetSize > 0 );  

        if ( pAvatarAssetBuffer != NULL )
        {            
            DWORD dwBytesRead = 0;
            BOOL bSucceeded = ReadFile( hFile, pAvatarAssetBuffer, dwAvatarAssetSize, &dwBytesRead, 0 );
            //assert(bSucceeded && (dwBytesRead == dwAvatarAssetSize));        
            if( !bSucceeded )
            {
                return E_FAIL;
            }
        }
        CloseHandle( hFile );


    }  
    
    // Set the custom asset into the Avatar metadata
    HRESULT result = XAvatarSetCustomAsset(dwAvatarAssetSize, pAvatarAssetBuffer, 0, 0, &m_vAvatarData.m_metadata);
    if (result != S_OK)
    {
        DebugMsg("AvatarRenderer", "XAvatarSetCustomAsset failed." );        
        return E_FAIL;
    }

    // Reload the AvatarRenderer
	XAvatarCreateRenderer(	&m_vAvatarData.m_metadata,
							XAVATAR_COMPONENT_MASK_ALL,
                            XAVATAR_SHADOW_SIZE_SMALL,
                            XAVATAR_MIPMAP_RECOMMENDED_BUFFER_SIZE,
                            0,
                            &m_vAvatarData.m_pRenderer );

	

	//HRESULT retVal = m_vAvatarData.m_pRenderer->GetStatus();

    // Data no longer needed now that the renderer is loaded so discard.
    XMemFree( pAvatarAssetBuffer, MAKE_XALLOC_ATTRIBUTES( 0,
                                                      TRUE,
                                                      TRUE,
                                                      FALSE,
                                                      SAMPLE_ALLOCATOR_ID,
                                                      XALLOC_ALIGNMENT_16,
                                                      XALLOC_MEMPROTECT_READWRITE,
                                                      FALSE,
                                                      XALLOC_MEMTYPE_HEAP));
													  
	*/
    return S_OK;    
}

HRESULT CScnTeamFSD::LoadAnimationFromFile( LPCTSTR szAnimFilename, LPXAVATARANIMATION* ppAnim )
{
    HANDLE hAnimFile                = NULL;
    BYTE* pAnimBuffer               = NULL;
    DWORD dwBytesRead               = 0;
    DWORD dwFileSize                = 0; 

    hAnimFile = CreateFile( szAnimFilename,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if( hAnimFile != INVALID_HANDLE_VALUE )
    {  
        // Load the file into a memory buffer
        dwFileSize = GetFileSize(hAnimFile, NULL);
        pAnimBuffer = reinterpret_cast<BYTE*>( XMemAlloc( dwFileSize, 
                                                          MAKE_XALLOC_ATTRIBUTES( 0,
                                                                                  TRUE,
                                                                                  TRUE,
                                                                                  FALSE,
                                                                                  SAMPLE_ALLOCATOR_ID,
                                                                                  XALLOC_ALIGNMENT_16,
                                                                                  XALLOC_MEMPROTECT_READWRITE,
                                                                                  FALSE,
                                                                                  XALLOC_MEMTYPE_HEAP)));

        // Create the animation assets from the file buffer
        if ( pAnimBuffer != NULL )
        {
            BOOL bSucceeded = ReadFile(hAnimFile, pAnimBuffer, dwFileSize, &dwBytesRead, NULL);
            //assert(bSucceeded && (dwBytesRead == dwFileSize));
            if( bSucceeded && (dwBytesRead == dwFileSize) )
            {
        
                XAvatarLoadAnimationFromBuffer( dwBytesRead, pAnimBuffer, ppAnim );
            }            
        }

        CloseHandle( hAnimFile );
    }
    else
    {
        return E_FAIL;
    }

    return S_OK;
}


void CScnTeamFSD::RenderAvatar( void )
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
	pD3DDevice->BeginTiling( 0, ARRAYSIZE(g_SmallResTiles), g_SmallResTiles, &clearColor, 1, 0 );

	m_vAvatarData.m_pRenderer->Render(pD3DDevice, m_vAvatarData.m_matWorld, m_matView, m_matProj);
	pD3DDevice->EndTiling( 0, NULL, m_pTexture, NULL, 1, 0, NULL );    	

	// Restore original RenderTargets and Stencils
	pD3DDevice->SetRenderTarget( 0, pOriginalRenderTarget );
	pD3DDevice->SetDepthStencilSurface(pOriginalDepthStencil);

}

void CScnTeamFSD::FreeAvatarResources( void )
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

}