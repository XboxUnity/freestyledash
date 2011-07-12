#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Arena/ArenaManager.h"

class CXlinkPlayerList :public CXuiListImpl
{
private :
	// Private variables
	ARENAITEM m_pArenaInfo;
	int m_nCurrentListSize;
	bool bIsLoaded, bInitializeList, bListChanged;

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_TIMER(OnTimer)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)

		KAI_ON_NOTIFY_OPPONENT_PING( OnOpponentPing )
		KAI_ON_NOTIFY_OPPONENT_LEAVE( OnOpponentLeave )
		KAI_ON_NOTIFY_OPPONENT_ENTER( OnOpponentEnter )
		KAI_ON_NOTIFY_ENTER_ARENA( OnEnterArena )
	XUI_END_MSG_MAP()

	CXlinkPlayerList();
	~CXlinkPlayerList();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);

	HRESULT OnEnterArena(KAIMessageOnEnterArena * pData, BOOL& bHandled);
	HRESULT OnOpponentPing(KAIMessageOnOpponentPing * pData, BOOL& bHandled);
	HRESULT OnOpponentLeave(KAIMessageOnOpponentLeave * pData, BOOL& bHandled);
	HRESULT OnOpponentEnter(KAIMessageOnOpponentEnter * pData, BOOL& bHandled);

	HRESULT LoadPlayerList(void);
public:
    XUI_IMPLEMENT_CLASS( CXlinkPlayerList, L"XlinkPlayerList", XUI_CLASS_LIST );
};
