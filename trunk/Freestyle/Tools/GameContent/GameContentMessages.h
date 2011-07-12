#pragma once
#include "../Managers/GameList/GameListItem.h"

#define VK_SHIFT_LB			0xFF01
#define VK_SHIFT_RB			0xFF02

typedef struct {
	GameListContentPtr pVec;
	bool bRetainSelection;
} FSDMessageSetListContent;

typedef struct {
	int nSortDirection;
	int nSortStyle;
	bool bFavoritesOnly;
} FSDMessageSetListSort;

typedef struct {
	GameListItem * pGameItem;
	int nCurrentIndex;
	int nGameListSize;
} FSDMessageRefreshContent;

typedef struct {
	int nSortStyle;
	int nSortDirection;
	bool bFavoritesOnly;
} FSDMessageNotifySortChange;

typedef struct {
	int nShiftKey;
} FSDMessageShiftKeyDown;

typedef struct {
	int nShiftKey;
} FSDMessageShiftKeyUp;

typedef struct {
	bool bPlay;
} FSDMessagePlayTransition;

#define XM_SET_LIST_CONTENT			XM_USER + 40
#define XM_SET_LIST_SORT			XM_USER + 41
#define XM_REFRESH_CONTENT			XM_USER + 42
#define XM_NOTIFY_SORT_CHANGE		XM_USER + 43
#define XM_SHIFT_KEY_DOWN			XM_USER + 44
#define XM_SHIFT_KEY_UP				XM_USER + 45

#define XM_PLAY_TRANSITION			XM_USER + 46

#define XUI_ON_PLAY_TRANSITION(MemberFunc)\
	if( pMessage->dwMessage == XM_PLAY_TRANSITION )\
	{\
		FSDMessagePlayTransition * pData = (FSDMessagePlayTransition *)pMessage->pvData; \
		return MemberFunc(pData, pMessage->bHandled); \
	}

#define XUI_ON_XM_SET_LISTCONTENT(MemberFunc)\
	if( pMessage->dwMessage == XM_SET_LIST_CONTENT)\
	{\
		FSDMessageSetListContent *pData = (FSDMessageSetListContent *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define XUI_ON_XM_SET_LISTSORT(MemberFunc)\
	if( pMessage->dwMessage == XM_SET_LIST_SORT)\
	{\
		FSDMessageSetListSort *pData = (FSDMessageSetListSort *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define XUI_ON_XM_REFRESH_CONTENT(MemberFunc)\
	if( pMessage->dwMessage == XM_REFRESH_CONTENT)\
	{\
		FSDMessageRefreshContent *pData = (FSDMessageRefreshContent *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define XUI_ON_XM_NOTIFY_SORT_CHANGE(MemberFunc)\
	if( pMessage->dwMessage == XM_NOTIFY_SORT_CHANGE)\
	{\
		FSDMessageNotifySortChange * pData = (FSDMessageNotifySortChange *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define XUI_ON_XM_SHIFT_KEYDOWN(MemberFunc)\
	if( pMessage->dwMessage == XM_SHIFT_KEY_DOWN)\
	{\
		FSDMessageShiftKeyDown * pData = (FSDMessageShiftKeyDown *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

#define XUI_ON_XM_SHIFT_KEYUP(MemberFunc)\
	if( pMessage->dwMessage == XM_SHIFT_KEY_UP)\
	{\
		FSDMessageShiftKeyUp * pData = (FSDMessageShiftKeyUp *)pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
	}

// Message Accesors
static __declspec(noinline) void FsdMessageSetListContent(XUIMessage *pMsg, FSDMessageSetListContent * pData, GameListContentPtr pVec, bool bRetainSelection)
{
    XuiMessage(pMsg, XM_SET_LIST_CONTENT);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->pVec = pVec;
	pData->bRetainSelection = bRetainSelection;
}

static __declspec(noinline) void FsdMessagePlayTransition(XUIMessage *pMsg, FSDMessagePlayTransition *pData, bool bPlay )
{
	XuiMessage( pMsg, XM_PLAY_TRANSITION);
	_XuiMessageExtra(pMsg, (XUIMessageData *)pData, sizeof(*pData));
	pData->bPlay = bPlay;
}

static __declspec(noinline) void FsdMessageSetListSort(XUIMessage *pMsg, FSDMessageSetListSort * pData, int nSortDirection, int nSortStyle)
{
	XuiMessage(pMsg, XM_SET_LIST_SORT);
	_XuiMessageExtra(pMsg, (XUIMessageData*) pData, sizeof(*pData));
	pData->nSortDirection = nSortDirection;
	pData->nSortStyle = nSortStyle;
}

static __declspec(noinline) void FsdMessageRefreshContent(XUIMessage *pMsg, FSDMessageRefreshContent * pData, GameListItem * pContent, int nCurrentIndex, int nGameListSize)
{
	XuiMessage(pMsg, XM_REFRESH_CONTENT);
	_XuiMessageExtra(pMsg, (XUIMessageData*) pData, sizeof(*pData));
	pData->pGameItem = pContent;
	pData->nCurrentIndex = nCurrentIndex;
	pData->nGameListSize = nGameListSize;
}

static __declspec(noinline) void FsdMessageNotifySortChange(XUIMessage *pMsg, FSDMessageNotifySortChange *pData, int nSortStyle, int nSortDirection, bool bFavoritesOnly)
{
	XuiMessage(pMsg, XM_NOTIFY_SORT_CHANGE);
	_XuiMessageExtra(pMsg, (XUIMessageData*) pData, sizeof(*pData));
	pData->nSortStyle = nSortStyle;
	pData->nSortDirection = nSortDirection;
	pData->bFavoritesOnly = bFavoritesOnly;
}

static __declspec(noinline) void FsdMessageShiftKeyDown(XUIMessage *pMsg, FSDMessageShiftKeyDown *pData, int nShiftKey)
{
	XuiMessage(pMsg, XM_SHIFT_KEY_DOWN);
	_XuiMessageExtra(pMsg, (XUIMessageData*) pData, sizeof(*pData));
	pData->nShiftKey = nShiftKey;
}

static __declspec(noinline) void FsdMessageShiftKeyUp(XUIMessage *pMsg, FSDMessageShiftKeyUp *pData, int nShiftKey)
{
	XuiMessage(pMsg, XM_SHIFT_KEY_UP);
	_XuiMessageExtra(pMsg, (XUIMessageData*) pData, sizeof(*pData));
	pData->nShiftKey = nShiftKey;
}

static __declspec(noinline) HRESULT FsdMsgPlayTransition(HXUIOBJ hObj, bool bPlay, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessagePlayTransition pPlayMsg;
	FsdMessagePlayTransition(&pMsg, &pPlayMsg, bPlay);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT FsdListSetContent(HXUIOBJ hObj, GameListContentPtr vListContent, bool bRetainSelection, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessageSetListContent pContentMsg;
	FsdMessageSetListContent(&pMsg, &pContentMsg, vListContent, bRetainSelection);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT FsdListSetSort(HXUIOBJ hObj, int nSortStyle, int nSortDirection, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessageSetListSort pSortMsg;
	FsdMessageSetListSort(&pMsg, &pSortMsg, nSortDirection, nSortStyle);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT FsdGameRefreshContent(HXUIOBJ hObj, GameListItem * pGameContent, int nCurrentIndex, int nGameListSize, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessageRefreshContent pRefreshMsg;
	FsdMessageRefreshContent(&pMsg, &pRefreshMsg, pGameContent, nCurrentIndex, nGameListSize);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT FsdNotifySortChange(HXUIOBJ hObj, int nSortStyle, int nSortDirection, bool bFavoritesOnly, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessageNotifySortChange pSortChangeMsg;
	FsdMessageNotifySortChange(&pMsg, &pSortChangeMsg, nSortStyle, nSortDirection, bFavoritesOnly);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT FsdShiftKeyDown(HXUIOBJ hObj, int nShiftKey, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessageShiftKeyDown pShiftKeyDown;
	FsdMessageShiftKeyDown(&pMsg, &pShiftKeyDown, nShiftKey);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}

static __declspec(noinline) HRESULT FsdShiftKeyUp(HXUIOBJ hObj, int nShiftKey, bool bBroadcast = true)
{
	XUIMessage pMsg;
	FSDMessageShiftKeyUp pShiftKeyUp;
	FsdMessageShiftKeyUp(&pMsg, &pShiftKeyUp, nShiftKey);
	if(bBroadcast)
		return XuiBroadcastMessage(hObj, &pMsg);
	else
		return XuiSendMessage(hObj, &pMsg);
}