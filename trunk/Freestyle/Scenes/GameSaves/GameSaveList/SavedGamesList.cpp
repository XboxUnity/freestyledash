#include "stdafx.h"

#include "SavedGamesList.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/XEX/Xbox360Container.h"
#include "../../../Tools/Managers/VariablesCache/VariablesCache.h"
#include "../../../Tools/Managers/GameList/GameListItem.h"
#include "../../../Tools/GameContent/GameContentManager.h"

CSavedGamesList::CSavedGamesList()
{
	InitializeCriticalSection(&lock);
	m_bListReady = false;
}

CSavedGamesList::~CSavedGamesList()
{ 
	vector<SaveGameContent*>::iterator iter;
	for(iter = m_SavedGameList.begin(); iter != m_SavedGameList.end(); ++iter)
		delete (*iter);

	DeleteCriticalSection(&lock); 
	Terminate(); 
}

HRESULT CSavedGamesList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	cItems = 0;
	pGameContent = NULL;
	m_sListSelection = GameContentManager::getInstance().getGameListSnapshot(false);
	if(m_sListSelection.CurrentGame == NULL)
		return S_OK;

	pGameContent = m_sListSelection.CurrentGame;

	isLoaded = false;
	CreateWorkerThread();

	return S_OK;
}

HRESULT CSavedGamesList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	return S_OK;
}

HRESULT CSavedGamesList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 1 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		if(isLoaded == true)
		{
			int mainItem = pGetSourceTextData->iItem;
			
			if((unsigned int)mainItem < m_SavedGameList.size()) {
				EnterCriticalSection(&lock);
				pGetSourceTextData->szText = m_SavedGameList.at(mainItem)->DisplayTitle.c_str();
				LeaveCriticalSection(&lock);
			}
		}

		bHandled = TRUE;
	}
	if( ( 2 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		if(isLoaded == true)
		{
			int subItem = pGetSourceTextData->iItem;

			if((unsigned int)subItem < m_SavedGameList.size()) {
				EnterCriticalSection(&lock);
				pGetSourceTextData->szText = m_SavedGameList.at(subItem)->SubTitle.c_str();
				LeaveCriticalSection(&lock);
			}
		}
		bHandled = TRUE;
	}

	if( ( 3 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		if(isLoaded == true)
		{
			int sizeItem = pGetSourceTextData->iItem;

			if((unsigned int)sizeItem < m_SavedGameList.size()) {
				EnterCriticalSection(&lock);
				pGetSourceTextData->szText = m_SavedGameList.at(sizeItem)->Size.c_str();
				LeaveCriticalSection(&lock);
			}
		}

		bHandled = TRUE;
	}
	
	
    return S_OK;
}

HRESULT CSavedGamesList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	// Display Icon Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEICON && pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEICON, &m_sListSelection.CurrentIndex, GameContentManager::getInstance().m_vDisplayListPointer);
		bHandled = true;
	}

	return S_OK;
}

HRESULT CSavedGamesList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = cItems;
	m_bListReady = true;
	bHandled = TRUE;
	return S_OK;
}

unsigned long CSavedGamesList::Process(void* parameter)
{
	while(!m_bListReady){
		Sleep(0);
	}

	if(isLoaded == false)
	{
		HRESULT hr = CreateSavedGamesList();
		if(hr == S_OK){
			isLoaded = true;
		}
		else
			DebugMsg("DEBUG", "Thread Returned Unexpectedly");
	}


	return 0;
}

HRESULT CSavedGamesList::CreateSavedGamesList( void )
{

	HRESULT hr = NULL;
	int fileCount = 0;
	XUID xuid;
	m_SavedGameList.clear();

	hr = XUserGetXUID(0, &xuid);
	if(hr != ERROR_SUCCESS)
	{
		DebugMsg("SavedGamesList", "Player 1 is not Signed Into a valid profile:  Error - 0x%08X", hr);
		return 2;
	}

	ContentItemNew * pContent = pGameContent->ContentRef;
	DWORD dwTitleId = pContent->getTitleId();

	DebugMsg("SavedGamesList", "Current Title ID:  %X", dwTitleId);

	string strTitlePath = sprintfaA("hdd1:\\Content\\%I64X\\%X\\00000001", xuid, dwTitleId);
	
	// Check if the path exists
	if(!FileExistsA(strTitlePath)) {
		DebugMsg("SavedGamesList", "There are no saved games detected for this TitleID:  %X", dwTitleId);
		return 2;
	}

	// Start Scanning the path if it is valid
	WIN32_FIND_DATA findFileData;
	memset(&findFileData, 0, sizeof(WIN32_FIND_DATA));
	
	string SearchCMD = strTitlePath + "\\*";
	SearchCMD = str_replaceall(SearchCMD, "\\\\", "\\");

	HANDLE hFind = FindFirstFileA(SearchCMD.c_str(), &findFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		DebugMsg("SavedGamesList", "An Error Has Occurred.  Invalid Handle Value.");
	}

	do{
		saveContent = new SaveGameContent;
		
		// Store file related information
		string szFileName = findFileData.cFileName;
		string szFullPath = strTitlePath + "\\" + szFileName;
		
		saveContent->SaveGameFileName = szFileName;
		saveContent->SaveGamePath = szFullPath;
		saveContent->Size = sprintfaW(L"%d KB", findFileData.nFileSizeLow / 1024);

		// Store Content related information
		Xbox360Container savFile;
		savFile.OpenContainer(szFullPath);
		
		DWORD titleID;
		titleID = savFile.pContainerMetaData->ExecutionId.TitleID;
		saveContent->TitleID = sprintfaW(L"%X", titleID);

		string titleName = "";
		int titleLen = 0;
		saveContent->SubTitle = savFile.GetTitleName();

		string displayTitle = "";
		int displayLen = 0;
		saveContent->DisplayTitle = savFile.GetDisplayName();
		
		savFile.CloseContainer();

		m_SavedGameList.push_back(saveContent);
		fileCount++;
	}while (FindNextFileA(hFind, &findFileData));
	
	FindClose(hFind);

	cItems = m_SavedGameList.size();

	if(fileCount == 1)
		Sleep(15);

	InsertItems(0,cItems);

	SetCurSel( 0 );

	return S_OK;
}
