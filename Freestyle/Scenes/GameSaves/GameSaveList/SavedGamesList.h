#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Threads/WorkerThread.h"
#include "../../../Tools/Managers/GameList/GameListItem.h"
#include "../../../Tools/GameContent/GameContentMessages.h"
#include "../../../Tools/GameContent/GameContentTypes.h"

typedef struct 
{
	string SaveGamePath;
	string SaveGameFileName;
	wstring DisplayTitle;
	wstring SubTitle;
	wstring TitleID;
	wstring Size;
	wstring ImagePath;
	bool showImage;
} SaveGameContent;

class CSavedGamesList :public CXuiListImpl, public WorkerThread
{
private :

	CRITICAL_SECTION lock;

	GAMECONTENT_LIST_STATE m_sListSelection;
	bool m_bListReady;
	int m_nListSize;


	GameListItem * pGameContent;

	bool isLoaded;
	bool validGame;
	wstring curTitleID;
	int cItems;
	vector<SaveGameContent*> m_SavedGameList;
	SaveGameContent * saveContent;

public:
    XUI_IMPLEMENT_CLASS( CSavedGamesList, L"SavedGamesList", XUI_CLASS_LIST );

	unsigned long Process(void* parameter);

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
	XUI_END_MSG_MAP()

	CSavedGamesList();
	~CSavedGamesList();

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT CreateSavedGamesList( void );
};
