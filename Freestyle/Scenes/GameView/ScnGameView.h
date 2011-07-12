#pragma once
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Managers/Skin/SkinXMLReader/SkinXMLReader.h"
#include "../../Tools/GameContent/GameContentMessages.h"
#include "../../Tools/Managers/GameList/GameListItem.h"
#include "../../Tools/GameContent/GameContentTypes.h"

class CScnGameView : public CXuiTabSceneImpl, SkinXMLReader
{
private:

	void InitializeGameXuiControls( void );
	void InitializeSceneXuiControls( void );
	void ReadSettings( void );

	wstring GetSceneTitle( BOOL showingFavs = FALSE );
	LPCWSTR GetSortStyleText( GAMELIST_SORTSTYLE nSortStyle);
	LPCWSTR GetSortDirectionText( GAMELIST_SORTDIRECTION nSortDirection );

	struct {
		bool m_bAutoHideReleaseDate;
		bool m_bUseCurrentSortType;
		bool m_bBackgroundVisible;
		bool m_bShowingFavorites;
		bool m_bLeftShiftHeld;
		bool m_bRightShiftHeld;
		WORD m_wRightShiftKey;
		WORD m_wLeftShiftKey;
		wstring m_szListCounterFormat;
		string szContentType; 
		CONTENT_ITEM_TAB m_sContentType;
		int nContentType;
		int nCurrentIndex;
		int nGameListSize;
		GAMELIST_SORTSTYLE nSortStyle;
		GAMELIST_SORTDIRECTION nSortDirection;
		vector<CXuiElement> m_vControlsVector;
	} GameViewSettings;

	bool bRBHeld;

	struct{
		CXuiTextElement m_GameDescription; 
		CXuiTextElement m_GameTitle;	
		CXuiTextElement m_GameGenre;
		CXuiTextElement m_GameReleaseDate; 
		CXuiTextElement m_GameRating; 
		CXuiTextElement m_GameRaters; 
		CXuiTextElement m_GameDeveloper;
		CXuiTextElement m_GameReleaseDateCaption; 
		CXuiVideo m_GamePreviewVideo; 
		CXuiVideo m_GameFullscreenVideo;
	} GameXuiControls;

	struct{	
		bool hasDescription; bool hasTitle; bool hasGenre;
		bool hasPreviewVideo; bool hasFullScreenVideo;
		bool hasReleaseDate; bool hasReleaseDateCaption; bool hasRating; bool hasRaters; bool hasDeveloper;
	} GameXuiControlState;

	struct {
		CXuiControl m_ButtonToggleSortStyle;
		CXuiControl m_ButtonToggleSortDirection;
		CXuiControl m_ButtonToggleSubtitle;
		CXuiControl m_ButtonToggleFavorite;
		CXuiControl m_ButtonShowOptions;
		CXuiControl m_ButtonShowFavorites;
		CXuiControl m_ButtonAdjustRatingUp;
		CXuiControl m_ButtonAdjustRatingDown;
		CXuiControl m_ButtonShiftLB;
		CXuiControl m_ButtonShiftRB;
		CXuiControl m_ButtonLaunchGame;
		CXuiControl m_ButtonToggleBackground;
		CXuiControl m_ButtonBack;
		CXuiTextElement m_TextSceneCaption;
		CXuiTextElement m_TextSceneSubCaption;
		CXuiTextElement m_TextGameListCounter;
		CXuiTextElement m_TextPrevSortStyle;
		CXuiTextElement m_TextCurrentSortStyle;
		CXuiTextElement m_TextNextSortStyle;
		CXuiTextElement m_TextPrevSortDirection;
		CXuiTextElement m_TextCurrentSortDirection;
		CXuiTextElement m_TextNextSortDirection;
	} SceneXuiControls;

	struct{
		bool hasButtonToggleSortStyle; bool hasButtonToggleSortDirection; bool hasButtonToggleSubtitle;
		bool hasButtonToggleFavorite; bool hasButtonShowOptions; bool hasButtonShowFavorites; bool hasButtonBack;
		bool hasButtonAdjustRatingUp; bool hasButtonAdjustRatingDown; bool hasButtonShiftLB; 
		bool hasButtonShiftRB; bool hasButtonLaunchGame; bool hasButtonToggleBackground;
		bool hasTextSceneCaption; bool hasTextPrevSortStyle; bool hasTextCurrentSortStyle; bool hasTextNextSortStyle;
		bool hasTextPrevSortDirection; bool hasTextCurrentSortDirection; bool hasTextNextSortDirection;
		bool hasTextGameListCounter; bool hasTextSceneSubCaption;
	} SceneXuiControlState;

	// Pointers
	GameListItem * pCurrentGame;

	// Methods to set game information
	void SetTitle(GameListItem& pContent);
	void SetDescription(GameListItem& pContent);
	void SetGenre(GameListItem& pContent);
	void SetReleaseDate(GameListItem& pContent);
	void SetRating(GameListItem& pContent);
	void SetRaters(GameListItem& pContent);
	void SetDeveloper(GameListItem& pContent);

	// Routines used in setting the different text elements 
	void SetSceneTitle(  BOOL showingFavs = FALSE );
	void SetSceneSubTitle( BOOL showingFavs = FALSE );
	void SetSortStyleCaptions(GAMELIST_SORTSTYLE nCurrentSortStyle);
	void SetSortDirectionCaptions(GAMELIST_SORTDIRECTION nCurrentSortDirection);
	void SetFavoritesButtonCaption(BOOL showingFavorites);
	void SetGameListCounter(int nCurrentIndex, int nListSize);
	void ToggleBackground( void );

	int nStartFrame;
	int nEndFrame;
	bool bHasFrames;

	wstring sceneCaption;

public:

	CScnGameView();
	~CScnGameView();

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress ) 
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
		XUI_ON_XM_REFRESH_CONTENT( OnRefreshContent )
		XUI_ON_XM_NOTIFY_SORT_CHANGE( OnNotifySortChange )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_KEYUP( OnKeyUp )
		XUI_ON_XM_SHIFT_KEYDOWN( OnShiftKeyDown )
		XUI_ON_XM_SHIFT_KEYUP( OnShiftKeyUp )
		XUI_ON_PLAY_TRANSITION( OnPlayTransition )
	XUI_END_MSG_MAP()

	XUI_IMPLEMENT_CLASS(CScnGameView, L"ScnGameView", XUI_CLASS_TABSCENE);

    HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnRefreshContent(FSDMessageRefreshContent *pRefreshContent, BOOL& bHandled);
	HRESULT OnNotifySortChange(FSDMessageNotifySortChange *pNotifySortChange, BOOL& bHandled);
	HRESULT OnShiftKeyUp(FSDMessageShiftKeyUp * pShiftKeyUp, BOOL& bHandled);
	HRESULT OnShiftKeyDown(FSDMessageShiftKeyDown * pShiftKeyDown, BOOL& bHandled);
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnPlayTransition(FSDMessagePlayTransition *pData, BOOL& bHandled);

};