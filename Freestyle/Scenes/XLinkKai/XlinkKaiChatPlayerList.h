#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Chat/ChatManager.h"

class CXlinkChatPlayerList :public CXuiListImpl
{
private :
	ChatPlayer m_pChatPlayers;
	int m_nCurrentListSize;

	// Private Variables
	bool bIsLoaded;
	bool bInitializeList;
	bool bListChanged;

private:
	HRESULT LoadPlayerList( void );

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_TIMER(OnTimer)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		// Kai Messages
		KAI_ON_NOTIFY_JOINS_CHAT( OnJoinsChat )
		KAI_ON_NOTIFY_LEAVES_CHAT( OnLeavesChat )
	XUI_END_MSG_MAP()

	CXlinkChatPlayerList();
	~CXlinkChatPlayerList();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	// Kai Callbacks
	HRESULT OnJoinsChat(KAIMessageOnJoinsChat * pData, BOOL& bHandled);
	HRESULT OnLeavesChat(KAIMessageOnLeavesChat * pData, BOOL& bHandled);

public:
    XUI_IMPLEMENT_CLASS( CXlinkChatPlayerList, L"XlinkChatPlayerList", XUI_CLASS_LIST );
};
