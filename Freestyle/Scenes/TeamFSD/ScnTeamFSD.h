#pragma once
#include "../../Application/FreestyleApp.h"
#include "../../Tools/Debug/Debug.h"

#define TEXTURE_RES_X	512	
#define TEXTURE_RES_Y	512

#define TILE_DIM_X		512
#define TILE_DIM_Y		256

#define ANIMATION_COUNT	2
#define SAMPLE_ALLOCATOR_ID		0

class CScnTeamFSD : public CXuiTabSceneImpl, public iRenderLoopObserver
{
private:

		// This struct holds all of the vital information regarding our players avatar
	struct AvatarData	{
		XUID						m_xuid;
		XAVATAR_METADATA			m_metadata;
		LPXAVATARANIMATION			s_pAnimations[ ANIMATION_COUNT ];
		BOOL						s_bAnimationsLoaded;
		LPXAVATARRENDERER			m_pRenderer;
		XAVATAR_WAITING_EFFECT		m_WaitingEffect;
		BOOL						m_bRenderWaitingEffect;
		BOOL						m_bWaitingEffectFadeOut;
		XMMATRIX					m_matWorld;
	};

	CXuiTextElement m_MemberIndex;
	CXuiTextElement m_MemberName;

	bool hasMemberIndex, hasMemberName;

	int m_nMemberIndex;
	HXUIBRUSH m_hBrush;

	// Timer used to determine correct animation frame.
	ATG::Timer m_Timer;

    // View parameters
    XMVECTOR                    m_vEyePt;
    XMVECTOR                    m_vUp;
    XMMATRIX                    m_matView;
    XMMATRIX                    m_matProj; 

	// Avatar State Variables
	DWORD		dwAnimationIndex;
	BOOL		bIsRendered;
	bool		bSafeToFree;

	// Direct3D Textures and Surfaces
	IDirect3DTexture9 *			m_pTexture;
	IDirect3DSurface9 *			m_pRenderTarget;
	IDirect3DSurface9 *			m_pDepthStencil;

    // Data Structure for loaded Avatar
	AvatarData					m_vAvatarData;

	HRESULT LoadCustomAvatarAsset();
	HRESULT LoadAnimationFromFile( LPCTSTR szAnimFilename, LPXAVATARANIMATION* ppAnim );
	void FreeAvatarResources( void );
	void InitializeChildren( void );
	void InitializeAvatarScene( void );
	void LoadLocalAvatar( void );
	void LoadAvatarAnimations( void );
	void UpdateAvatar( void );
	void RenderAvatar( void );

protected:
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);

public:
	CScnTeamFSD();
	~CScnTeamFSD();

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
    XUI_END_MSG_MAP()  

    XUI_IMPLEMENT_CLASS( CScnTeamFSD, L"ScnTeamFSD", XUI_CLASS_SCENE );

	// iRenderLoopInjector Implementation
	virtual void handleOnFrameUpdate()
	{
		UpdateAvatar();
		RenderAvatar();
		bSafeToFree = true;
	}

	virtual void handleOnFramePreRender() {} 
	virtual void handleOnFrameRenderPreXUI() {}
	virtual void handleOnFrameRenderPostXUI(){}
	virtual void handleOnFramePostRender() {} 
};