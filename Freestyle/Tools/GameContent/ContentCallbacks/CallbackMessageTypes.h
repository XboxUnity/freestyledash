#pragma once
#include "../../Managers/GameList/GameListItem.h"
#include "../../ContentList/ContentItemNew.h"

// defines for message handler to make changing variables a little easier
#define CON_THREAD_CONTENT_PRIORITY		THREAD_PRIORITY_NORMAL
#define CON_THREAD_LOADER_PRIORITY		THREAD_PRIORITY_NORMAL
#define CON_THREAD_CONTENT_CORE			CPU3_THREAD_1
#define CON_THREAD_LOADER_CORE			CPU3_THREAD_1
#define CON_THREAD_TERMINATE_TIMEOUT	10000
#define CON_THREAD_SLEEP_PAUSE			250
#define CON_THREAD_SLEEP_IDLE			10

typedef enum _MESSAGE_TYPE {
	UnknownMessage = 0,
	ContentAdded,
	ContentMassAdd,
	ContentUpdated,
	CacheLoaded,
	DatabaseLoaded,
	ScanCompleted,
	LoadAssets
} CallbackMessageType;

class ContentMessageData {
public:
	CallbackMessageType nMsgType;
	ContentItemNew * pContentItem;
	vector<ContentItemNew*> pVec;
	DWORD dwMessageFlags;
	ContentMessageData() {
		nMsgType = UnknownMessage;
		pContentItem = NULL;
		dwMessageFlags = 0;
	}
};

class LoaderMessageData {
public:
	CallbackMessageType nMsgType;
	GameListItem * pGameContent;
	ContentItemNew * pContentItem;
	DWORD dwMessageFlags;
	LoaderMessageData() {
		nMsgType = UnknownMessage;
		pContentItem = NULL;
		pGameContent = NULL;
		dwMessageFlags = 0;
	}
};
