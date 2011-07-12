#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/Arena/ArenaManager.h"

class CXlinkBuddyList :public CXuiListImpl
{
private :
	int m_currentListSize;
	bool isLoaded;
	wstring temp;

	int m_nCurrentListSize;
	bool bIsLoaded, bInitializeList, bListChanged;

	std::map<string, CKaiPlayer> m_pBuddyList;
	vector<string> m_BuddyNames;

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_TIMER(OnTimer)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)

		KAI_ON_NOTIFY_CONTACT_PING( OnContactPing )
		KAI_ON_NOTIFY_CONTACT_ADD( OnContactAdd )
		KAI_ON_NOTIFY_CONTACT_REMOVE( OnContactRemove )
		KAI_ON_NOTIFY_CONTACT_ONLINE( OnContactOnline )
		KAI_ON_NOTIFY_CONTACT_OFFLINE( OnContactOffline )

	XUI_END_MSG_MAP()

	CXlinkBuddyList();
	~CXlinkBuddyList();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);

	HRESULT OnContactPing(KAIMessageOnContactPing * pData, BOOL& bHandled);
	HRESULT OnContactAdd(KAIMessageOnContactAdd * pData, BOOL& bHandled);
	HRESULT OnContactRemove(KAIMessageOnContactRemove * pData, BOOL& bHandled);
	HRESULT OnContactOnline(KAIMessageOnContactOnline * pData, BOOL& bHandled);//May not use
	HRESULT OnContactOffline(KAIMessageOnContactOffline * pData, BOOL& bHandled);//May not use

	HRESULT LoadBuddyList();

	CRITICAL_SECTION lock;


public:
    XUI_IMPLEMENT_CLASS( CXlinkBuddyList, L"XlinkBuddyList", XUI_CLASS_LIST );
};
