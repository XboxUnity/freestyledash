#pragma once

#include <d3d9.h>
#include <d3dx9math.h>
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Monitors/SignIn/SignInMonitor.h"
#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../Abstracts/HideableScene/HideableScene.h"
#include "../../Tools/Texture/TextureItem/TextureItem.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/ContentList/ContentItemNew.h"

#define TILEHEIGHT 64
#define	TILEWIDTH 64
#define MAX_GAMERTAG_LENGTH 100
#define MAX_RECENTGAME 4

#define TILEID	0x8000

#define USE_RECENT_FROM_DB

// Image Presenter Data Association IDs
#define PROFILEIMG_GAMERPIC			0
#define PROFILEIMG_RECENTGAMEBASE	101

typedef struct {
	DWORD dwTitleId;
	FILETIME ftLastLoaded;
} PlayedTitle;

class CScnProfile:public HideableScene,public iSignInObserver
{
protected :
	struct {
		int m_nPlayerIndex;
		bool m_bProfileLoggedIn;
		bool m_bRenderRecent;
		bool m_bRenderGamerPic;
		bool m_bGamerPicError;
		bool m_bRecentGameError[MAX_RECENTGAME];
		wstring szGamerTag;
		wstring szGamerScore;
	} ProfileSceneSettings;

	struct {
		TextureItem m_GamerPictureTexture;
		TextureItem m_RecentGameTexture[MAX_RECENTGAME];
	} Textures ;

	struct {
		CXuiElement m_GamerPic;
		CXuiTextElement m_GamerTag;
		CXuiTextElement m_GamerScore;
		CXuiImageElement m_GamerScoreIcon;
		CXuiElement m_RecentGame[MAX_RECENTGAME];
		CXuiTextElement m_SignedIntoKai;
		CXuiTextElement m_SignedIntoKaiLabel;
	} SceneXuiControls;


	vector<RecentlyPlayed> vTitlesPlayed;
	vector<ContentItemNew*> vRecentContentItems;

	struct {
		bool hasRecentGame[MAX_RECENTGAME]; bool hasGamerPic; bool hasGamerTag; bool hasGamerScore; bool hasGamerScoreIcon; bool hasKaiSignedIn; bool hasKaiSignedInLabel;
	} SceneXuiControlState;

public :
	// XUI Message Map
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
		//Kai Login Info
		KAI_ON_NOTIFY_ACCOUNT_LOGGED_IN( OnNotifyAccountLoggedIn )
		KAI_ON_NOTIFY_DEINITIALISE( OnNotifyDeinitialise )
	XUI_END_MSG_MAP()

	// Constructor and Desconstructor
	CScnProfile();
	~CScnProfile();

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS(CScnProfile, L"ScnProfile", XUI_CLASS_SCENE);

	// Function Prototypes
	HRESULT ReadSettings( void );
	HRESULT ReadGamerPicTexture( void );
	HRESULT ReadRecentGameTexture( void );
	HRESULT ReadRecentGameTextureFromDB( void );

    HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	// Kai Specific Messages and Notifications
	HRESULT OnNotifyAccountLoggedIn( KAIMessageOnAccountLoggedIn * pOnAccountLoggedIn, BOOL& bHandled );
	HRESULT OnNotifyDeinitialise( KAIMessageOnDeinitialise * pOnDeinitialise, BOOL& bHandled );
	HRESULT CreateRecentGameTextures(int nGameIndex);
	HRESULT CreateGamerPicTexture();
	HRESULT InitializeChildren( void );
	void Update();

	//iSignInObserver implementation
	virtual void handleStatusChange() { Update(); }
};

bool sortPlayeTitleLastLoaded(PlayedTitle left, PlayedTitle right);