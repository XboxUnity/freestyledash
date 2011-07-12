#pragma once
#include "../../Application/FreestyleApp.h"
#include "../../Tools/Debug/Debug.h"
#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../../Tools/Monitors/SignIn/SignInMonitor.h"

#define TEXTURE_RES_X	512	
#define TEXTURE_RES_Y	512

#define TILE_DIM_X		512
#define TILE_DIM_Y		256

#define AVATAR_PRESENTER_INDEX	0
#define ANIMATION_COUNT			2
#define PLAYER_COUNT			5
#define SAMPLE_ALLOCATOR_ID		0

class CScnAvatar : public ConfigurableScene, public iRenderLoopObserver, public iSignInObserver
{
private:

	// This struct holds all of the vital information regarding our players avatar
	struct AvatarData	{
		XUID						m_PlayerXuid;
		XAVATAR_METADATA			m_Metadata;
		LPXAVATARANIMATION			s_pAnimations[ ANIMATION_COUNT ];
		BOOL						s_bAnimationsLoaded;
		LPXAVATARRENDERER			m_pRenderer;
		XAVATAR_WAITING_EFFECT		m_WaitingEffect;
		BOOL						m_bRenderWaitingEffect;
		BOOL						m_bWaitingEffectFadeOut;
		XMMATRIX					m_matWorld;
	};

	// Xui Controls and Variables to contain the state of each control
	CXuiElement m_AvatarPresenter;
	CXuiImageElement m_AvatarShadow;
	CXuiImageElement m_AvatarSilhouette;
	CXuiProgressBar m_AutoRotate;
	CXuiSlider m_RotationSlider;
	
	bool hasAvatarPresenter, hasAvatarShadow, hasAvatarSilhouette;
	bool hasAutoRotate, hasRotationSlider;
	HXUIBRUSH m_hAvatarBrush;

	// Timer used to determine correct animation frame.
	ATG::Timer m_Timer;

    // View vectors and matrices
    XMVECTOR                    m_vEyePt;
    XMVECTOR                    m_vUp;
    XMMATRIX                    m_matView;
    XMMATRIX                    m_matProj; 

	// Avatar State Variables
	bool		assetLoaded;
	bool		bPlayWalkInAnim;
	DWORD		lastInputPacket;
	int			nAutoRotateMax;
	int			nAutoRotateMin;
	DWORD		dwAnimationIndex;
	DWORD		dwShadowState;
	DWORD		dwPlayerId;
	BOOL		bIsRendered;
	BOOL		bRenderHighRes;
	bool		bSafeToFree;
	bool		bAutoRotate;
	bool		bHideSceneOnError;
	bool		bPlayerOnline;
	bool		bStateChanged;
	float		fAngle;

	// Direct3D Textures and Surfaces
	IDirect3DTexture9 *			m_pTexture;
	IDirect3DSurface9 *			m_pRenderTarget;
	IDirect3DSurface9 *			m_pDepthStencil;

    // Data Structure for loaded Avatar
	AvatarData					m_vAvatarData;

	// Avatar Functions
	HRESULT LoadCustomAvatarAsset();
	HRESULT LoadAnimationFromFile( LPCTSTR szAnimFilename, LPXAVATARANIMATION* ppAnim );
	void FreeAvatarResources( void );
	void InitializeChildren( void );
	void ReadSettings( void );
	void InitializeAvatarScene( void );
	void LoadLocalAvatar( void );
	void LoadAvatarAnimations( void );
	void UpdateAvatar( void );
	void RenderAvatar( void );
	bool CheckUserSettings(void);

	void ShowMissingAvatarElement(void);
	void HideMissingAvatarElement(void);

protected:
	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);

public:
	CScnAvatar();
	~CScnAvatar();

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
    XUI_END_MSG_MAP()  

    XUI_IMPLEMENT_CLASS( CScnAvatar, L"ScnAvatar", XUI_CLASS_SCENE );

	virtual void handlePlayerSignedIn( DWORD dwPlayerIndex, XUID newPlayerXuid )
	{
		if(dwPlayerIndex==dwPlayerId) {
			bPlayerOnline = true;
			bStateChanged = true;
		}
	}
	virtual void handlePlayerSignedOut( DWORD dwPlayerIndex, XUID oldPlayerXuid )
	{
		if(dwPlayerIndex==dwPlayerId) {
			bPlayerOnline = false; 
			bStateChanged = true;
		}
	}
	virtual void handlePlayerChanged( DWORD dwPlayerIndex, XUID newPlayerXuid, XUID oldPlayerXuid )
	{
		if(dwPlayerIndex==dwPlayerId) {
			bPlayerOnline = true;
			bStateChanged = true;
		}
	}

	void UserSignedIn()
	{
		LoadLocalAvatar();
		HideMissingAvatarElement();
	}

	void UserSignedOut()
	{
		FreeAvatarResources();
		ShowMissingAvatarElement();
	}

	// iRenderLoopInjector Implementation
	virtual void handleOnFrameUpdate()
	{
		UpdateAvatar();
		RenderAvatar();
		bSafeToFree = true;
	}
};