#pragma once
#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Threads/ThreadLock.h"
#include "../../../Tools/GameContent/GameContentTypes.h"
#include "../../../Tools/GameContent/GameContentMessages.h"

class CGamesList :public CXuiListImpl
{
private :
	// Private Variable that holds the current list information
	GameListContentPtr m_vListContent;

	// Private Variables that holds the current list content type
	int m_nContentType;
	string m_szContentType;

	// Private Variable to hold the current list text
	LPCWSTR subTitleText;

	// Private variables to hold current list information
	int m_nListSize;
	GAMELIST_SORTSTYLE m_nSortStyle;
	GAMELIST_SORTDIRECTION m_nSortDirection;
	GAMELIST_SUBTITLEVIEW m_nSubtitleView;
	int m_nCurrentIndex;
	int m_nPreviousIndex;
	GameListItem * m_CurrentGame;
	GameListItem * m_PreviousGame;

	// Private variables to assist in threading and state changes
	ThreadLock m_Lock;
	bool flag;

public:
	// Public Routines for Accessing List Content
	void RefreshCurrentItem();
	GameListItem* GetCurrentGame( void );

	// Public Accessors for List Properites
	void SetListSize(int nListSize) { m_nListSize = nListSize; }
	int GetListSize( void ) { return m_nListSize; }
	
	void SetCurrentSortStyle( GAMELIST_SORTSTYLE nSortStyle ) { m_nSortStyle = nSortStyle; }
	void SetCurrentSortDirection( GAMELIST_SORTDIRECTION nSortDirection ) { m_nSortDirection = nSortDirection; }
	void SetCurrentSubtitleView( GAMELIST_SUBTITLEVIEW nSubtitleView ) { m_nSubtitleView = nSubtitleView; }
	
	GAMELIST_SORTSTYLE GetCurrentSortStyle( void ) { return m_nSortStyle; }
	GAMELIST_SORTDIRECTION GetCurrentSortDirection( void ) { return m_nSortDirection; }
	GAMELIST_SUBTITLEVIEW GetCurrentSubtitleView( void ) { return m_nSubtitleView; }

	// Construstor/Deconstructor
	CGamesList();
	~CGamesList();

	// Implement Class for Class Overrides
    XUI_IMPLEMENT_CLASS( CGamesList, L"GamesList", XUI_CLASS_LIST );

	// XUI MessageMap
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_GET_SOURCE_TEXT( OnGetSourceDataText )
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
		XUI_ON_XM_GET_ITEMCOUNT_ALL( OnGetItemCountAll )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_SORT_CHANGE( OnNotifySortChanged )
		XUI_ON_XM_SET_CURSEL( OnSetCurSel )
		XUI_ON_XM_SET_LISTCONTENT( OnSetListContent )
	XUI_END_MSG_MAP()

	// XUI MessageMap Methods for GamesLIST
	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnSetCurSel(XUIMessageSetCurSel *pSetCurSelData, BOOL &bHandled );
	HRESULT OnSetListContent(FSDMessageSetListContent * pSetListContent, BOOL &bHandled );
	HRESULT OnNotifySortChanged(FSDMessageNotifySortChange * pNotifySortChange, BOOL &bHandled );
};
