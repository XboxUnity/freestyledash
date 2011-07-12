#include "stdafx.h"

#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Application/FreestyleApp.h"

#include "../../Tools/Plugin/PluginLoader.h"

#include "ScnGameControls.h"

const WCHAR * szRenameCaption = L"Change Game Title";
const WCHAR * szRenameDescription = L"Enter a new game title...";
const WCHAR * szDeleteCaption = L"Delete Game";
const WCHAR * szDeleteDescription = L"Are you sure you want to completely delete %s?\n\n Everything in path: %s Will be Deleted";
const WCHAR * szRemoveCaption = L"Remove Game From List";
const WCHAR * szRemoveDescription = L"Are you sure you want to remove the game below from your list? \n\nGame:  %s\nPath:  %s\n\nThis is not reversible and will require a database refresh to get back.";

CScnGameControls::CScnGameControls()
{
	// Initialize a couple of pointers
	pGameContent = NULL;
	pContentItem = NULL;
	kbBuffer = NULL;
	nMsgType = 0;
}

CScnGameControls::~CScnGameControls()
{
	// Clean up some memory
	if(szBtnOk != NULL) delete [] szBtnOk;

	if(kbBuffer != NULL) delete [] kbBuffer;
}

HRESULT CScnGameControls::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	InitializeChildren();

	// Grab a snapshot of the list from the GameContentManager
	listPack = GameContentManager::getInstance().getGameListSnapshot(false);

	// Prepare Buffer for Title Renaming
	kbBuffer = new WCHAR[200];
	szBtnOk = new LPCWSTR[2];
	szBtnOk[0] = L"OK";
	szBtnOk[1] = L"Cancel";

	// Set Scene to not display file progress
	ShowFileProgress(false);

	// Set Back to Main variable
	bBackToMain = false;

	pGameContent = listPack.CurrentGame;
	if(pGameContent == NULL || pGameContent->ContentRef == NULL)
		return S_FALSE;

	// Extract some common information from the ContentItem
	pContentItem = pGameContent->ContentRef;
	nGameType = pContentItem->GetItemTab();
	
	// Disable the appropriate move menus for the currently selected game
	DisableCurrentGameType(nGameType);

    return S_OK;
}


//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScnGameControls::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(hObjPressed == m_GameScreenshots) {
		// Launch the Game Screenshots Scene
		SkinManager::getInstance().setScene("screenshots.xur", *this, false);
		bHandled = true;

	} else if (hObjPressed == m_GameSaves) {
		// Launch the Game Saves Scene
		SkinManager::getInstance().setScene("savedgames.xur", *this, false);
		bHandled = true;

	} else if (hObjPressed == m_ManageTUs) {
		// Launch the Title Update Manager Scene
		SkinManager::getInstance().setScene("TitleUpdateManager.xur", *this, false);
		bHandled = true;

	} else if (hObjPressed == m_Achievements) {
		// Launch the Achievements Scene
		SkinManager::getInstance().setScene("Achievements.xur", *this, false);
		bHandled = true;

	} else if (hObjPressed == m_ManageTrainers) {
		// Launch the Trainers Scene
		if(PluginLoader::getInstance().IsLoaded() == TRUE) { 
			SkinManager::getInstance().setScene("Trainers.xur", this->m_hObj, false, "Trainers", NULL);
		}else {
			XNotifyQueueUICustom(L"This feature requires that you run the FSDPlugin.");
		}
		bHandled = true;
	} else if (hObjPressed == m_RenameGame) {
		// Initiate the Rename Process
		memset(&xoKeyboard, 0, sizeof(xoKeyboard));
		LPCWSTR szGameTitle = listPack.CurrentGame->Content.m_szTitle;
		// Launch Keyboard Scene to rename game
		XShowKeyboardUI(0, VKBD_DEFAULT, szGameTitle, szRenameCaption, szRenameDescription, 
			kbBuffer, 200, &xoKeyboard);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = true;

	} else if (hObjPressed == m_DeleteGame) {
		// Laucnh a Message Box for Game Deletion
//		string szGameTitle = wstrtostr(listPack.CurrentGame->Content.m_szTitle);
//		szTempString = sprintfaW(szDeleteDescription, szGameTitle.c_str());
		nMsgType = 1;
		if (listPack.CurrentGame->ContentRef->GetFileType() == CONTENT_FILE_TYPE_CONTAINER)
			szTempStringPath = strtowstr(listPack.CurrentGame->ContentRef->getRoot()) + strtowstr(listPack.CurrentGame->ContentRef->getPath());
		else 
			szTempStringPath = strtowstr(listPack.CurrentGame->ContentRef->getRoot()) + strtowstr(listPack.CurrentGame->ContentRef->getDirectory());
		szTempString = sprintfaW(szDeleteDescription, listPack.CurrentGame->Content.m_szTitle, szTempStringPath.c_str());
		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), szDeleteCaption,
			szTempString.c_str(), 2, szBtnOk, 1, NULL, NULL);
		bHandled = true;

	} else if (hObjPressed == m_MoveGame) {
		if(bIsNested) {
			m_GameControls.FindNamedFrame(L"MoveGame", &nFrameStart);
			m_GameControls.FindNamedFrame(L"EndMoveGame", &nFrameEnd);

			m_GameControls.PlayTimeline(nFrameStart, nFrameStart, nFrameEnd, false, false);
		} else {
			this->FindNamedFrame(L"MoveGame", &nFrameStart);
			this->FindNamedFrame(L"EndMoveGame", &nFrameEnd);

			this->PlayTimeline(nFrameStart, nFrameStart, nFrameEnd, false, false);
		}
		m_MoveXbox360.SetFocus(XUSER_INDEX_ANY);
		bBackToMain = true;
		m_Back.SetText(L"Cancel");
		bHandled = true;

	} else if (hObjPressed == m_Back) {
		if(bBackToMain)	{
			if(bIsNested) {
				m_GameControls.FindNamedFrame(L"BackToMain", &nFrameStart);
				m_GameControls.FindNamedFrame(L"EndBackToMain", &nFrameEnd);

				m_GameControls.PlayTimeline( nFrameStart, nFrameStart, nFrameEnd, false, false);
			} else {
				this->FindNamedFrame(L"BackToMain", &nFrameStart);
				this->FindNamedFrame(L"EndBackToMain", &nFrameEnd);

				this->PlayTimeline( nFrameStart, nFrameStart, nFrameEnd, false, false);
			}

			m_GameScreenshots.SetFocus(XUSER_INDEX_ANY);
			bBackToMain = false;
			m_Back.SetText(L"Back");
		} else {
			NavigateBack(XUSER_INDEX_ANY);
		}
		bHandled = true;

	}else if (hObjPressed == m_DownloadGame) {
		// Start downloading of assets to refresh
		if(CFreestyleApp::getInstance().hasInternetConnection() == false) {
			XNotifyQueueUICustom(L"No network connection detected.");
		} else {
			pContentItem->RefreshAssets();
		}
		bHandled = true;

	// Handle all the Content Moves
	} else if (hObjPressed == m_MoveXbox360) {
		ReloadContent(CONTENT_360);
		bHandled = true;
	} else if (hObjPressed == m_MoveXBLA) {
		ReloadContent(CONTENT_XBLA);
		bHandled = true;
	} else if (hObjPressed == m_MoveHomebrew) {
		ReloadContent(CONTENT_HOMEBREW); 
		bHandled = true;
	} else if (hObjPressed == m_MoveXbox1) {
		ReloadContent(CONTENT_XBOX1);
		bHandled = true;
	} else if (hObjPressed == m_MoveEmulators) {
		ReloadContent(CONTENT_EMULATOR);
		bHandled = true;
	}else if (hObjPressed == m_MoveNoSection) {
		nMsgType = 2;
		if (listPack.CurrentGame->ContentRef->GetFileType() == CONTENT_FILE_TYPE_CONTAINER)
			szTempStringPath = strtowstr(listPack.CurrentGame->ContentRef->getRoot()) + strtowstr(listPack.CurrentGame->ContentRef->getPath());
		else 
			szTempStringPath = strtowstr(listPack.CurrentGame->ContentRef->getRoot()) + strtowstr(listPack.CurrentGame->ContentRef->getDirectory());
		
		szTempString = sprintfaW(szRemoveDescription, listPack.CurrentGame->Content.m_szTitle, szTempStringPath.c_str() );
		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), szRemoveCaption,
			szTempString.c_str(), 2, szBtnOk, 1, NULL, NULL);
		bHandled = true;
	}
    return S_OK;
}

HRESULT CScnGameControls::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
    switch(pTimer->nId) {
		case TM_KEYBOARD:
			if(XHasOverlappedIoCompleted(&xoKeyboard)) {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(xoKeyboard.dwExtendedError == ERROR_SUCCESS) {
					pGameContent->ContentRef->setTitle(kbBuffer, TRUE);
					pGameContent->RefreshContentText();
				}				
			}
			break;
		case TM_DELETE:
			// Update file progress
			UpdateFileProgress();
			if(FileOperationManager::getInstance().isDone()) {
				// File Operation is complete
				KillTimer(TM_DELETE);
				m_Loading.SetShow(false);
				ReloadContent();
				NavigateBack(XUSER_INDEX_ANY);
				bHandled = TRUE;
			}
			break;
	};
    return S_OK;
}

HRESULT CScnGameControls::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{
	switch( pMsgBox->nButton ) {
        case 0:
			//Delete Content
			if(nMsgType == 1) 
			{
				string szAssetSource = "";
				if(pContentItem != NULL)
					szAssetSource = pContentItem->getAssetDirectory();

				if (FileExistsA(szAssetSource))
					FileOperationManager::getInstance().AddFolderOperation(szAssetSource, "", true);
			
				//Delete Game
				string szGameSource = "";

				if(pContentItem != NULL) {
					if (pContentItem->GetFileType() == CONTENT_FILE_TYPE_CONTAINER) {
						szGameSource = pContentItem->getRoot() + pContentItem->getPath();
						if (FileExistsA(szGameSource))
							FileOperationManager::getInstance().AddFileOperation(szGameSource, "", true);
					} else {
						szGameSource = pContentItem->getRoot() + pContentItem->getDirectory();
						if(FileExistsA(szGameSource))
							FileOperationManager::getInstance().AddFolderOperation(szGameSource, "", true);
					}
				}

				FileOperationManager::getInstance().DoWork();

				ShowFileProgress(true);
				m_GameTitle.SetText(listPack.CurrentGame->Content.m_szTitle);

				SetTimer(TM_DELETE, 50);
				nMsgType = 0;
			}
			else if ( nMsgType == 2 )
			{
				// Move Game
				ReloadContent(CONTENT_NONE); //No way to get them back yet
				nMsgType = 0;
			}

            break;
    }
    
    bHandled = TRUE;

	return S_OK;
}

HRESULT CScnGameControls::InitializeChildren( void )
{
	//Check if there is a child scn
	HRESULT hr = GetChildById(L"GameControls", &m_GameControls);
	bIsNested = hr == S_OK;

	if(bIsNested) // Contains Nested Scn
	{
		m_GameControls.GetChildById( L"GameScreenshots", &m_GameScreenshots );
		m_GameControls.GetChildById( L"GameSaves", &m_GameSaves );
		m_GameControls.GetChildById( L"RenameGame", &m_RenameGame );
		m_GameControls.GetChildById( L"DeleteGame", &m_DeleteGame );
		m_GameControls.GetChildById( L"MoveGame", &m_MoveGame );
		m_GameControls.GetChildById( L"RefreshArtwork", &m_DownloadGame );
		m_GameControls.GetChildById( L"MoveXbox360", &m_MoveXbox360 );
		m_GameControls.GetChildById( L"MoveXBLA", &m_MoveXBLA );
		m_GameControls.GetChildById( L"MoveHomebrew", &m_MoveHomebrew );
		m_GameControls.GetChildById( L"MoveXbox1", &m_MoveXbox1 );
		m_GameControls.GetChildById( L"MoveEmulators", &m_MoveEmulators );
		m_GameControls.GetChildById( L"MoveNoSection", &m_MoveNoSection);
		m_GameControls.GetChildById( L"TotalPercentComplete", &m_OverallPercent );
		m_GameControls.GetChildById( L"TotalProgressBar", &m_TotalProgressBar );
		m_GameControls.GetChildById( L"Loading", &m_Loading );
		m_GameControls.GetChildById( L"GameTitle", &m_GameTitle );
		m_GameControls.GetChildById( L"FileName", &m_FileName );
		m_GameControls.GetChildById( L"ManageTUs", &m_ManageTUs );
		m_GameControls.GetChildById( L"Achievements", &m_Achievements );
		m_GameControls.GetChildById( L"ManageTrainers", &m_ManageTrainers );
	}
	else
	{
		GetChildById( L"GameScreenshots", &m_GameScreenshots );
		GetChildById( L"GameSaves", &m_GameSaves );
		GetChildById( L"RenameGame", &m_RenameGame );
		GetChildById( L"DeleteGame", &m_DeleteGame );
		GetChildById( L"MoveGame", &m_MoveGame );
		GetChildById( L"RefreshArtwork", &m_DownloadGame );
		GetChildById( L"MoveXbox360", &m_MoveXbox360 );
		GetChildById( L"MoveXBLA", &m_MoveXBLA );
		GetChildById( L"MoveHomebrew", &m_MoveHomebrew );
		GetChildById( L"MoveXbox1", &m_MoveXbox1 );
		GetChildById( L"MoveEmulators", &m_MoveEmulators );
		GetChildById( L"Back", &m_Back);
		GetChildById( L"MoveNoSection", &m_MoveNoSection);
		GetChildById( L"TotalPercentComplete", &m_OverallPercent );
		GetChildById( L"TotalProgressBar", &m_TotalProgressBar );
		GetChildById( L"Loading", &m_Loading );
		GetChildById( L"GameTitle", &m_GameTitle );
		GetChildById( L"FileName", &m_FileName );
		GetChildById( L"ManageTUs", &m_ManageTUs );
		GetChildById( L"Achievements", &m_Achievements );
	}

	GetChildById( L"Back", &m_Back );
	
	return S_OK;
}

HRESULT CScnGameControls::DisableCurrentGameType(CONTENT_ITEM_TAB sCurrentTabId)
{
	if(sCurrentTabId == CONTENT_XBLA) {
		m_MoveXBLA.SetEnable(FALSE);

	} else if(sCurrentTabId == CONTENT_360) {
		m_MoveXbox360.SetEnable(FALSE);
		m_MoveXbox1.SetEnable(FALSE);

	} else if(sCurrentTabId == CONTENT_XBOX1) {
		m_MoveXbox1.SetEnable(FALSE);
		m_MoveXbox360.SetEnable(FALSE);

	} else if(sCurrentTabId == CONTENT_HOMEBREW) {
		m_MoveHomebrew.SetEnable(FALSE);

	} else if(sCurrentTabId == CONTENT_EMULATOR) {
		m_MoveEmulators.SetEnable(FALSE);

	} else {
		// Content Type Unknown
	}

	// Set the local tabid variable to global
	m_sTabId = sCurrentTabId;

	return S_OK;
}

HRESULT CScnGameControls::ReloadContent(CONTENT_ITEM_TAB sTabId)
{
	if(pContentItem == NULL)
		return S_FALSE;

	//Change Game Type
	pContentItem->MoveItem(sTabId);

	//Remove Game from current list
	if(pGameContent != NULL) {
		GameContentManager::getInstance().DeleteGameFromList(pGameContent);
		if (listPack.CurrentIndex != 0 || listPack.ListSize > 1) {  // nothing to set selection to if we are removing the last item
			//Set focus up one if last item in list is removed
			if(listPack.CurrentIndex == (listPack.ListSize - 1)) {
				GameContentManager::getInstance().m_cGameList->SetCurSel(listPack.CurrentIndex - 1);
			} else {
				GameContentManager::getInstance().m_cGameList->SetCurSel(listPack.CurrentIndex);
			}
		}
		m_Back.SetFocus(XUSER_INDEX_ANY);
		NavigateBack(XUSER_INDEX_ANY);
	}

	return S_OK;
}

HRESULT CScnGameControls::ReloadContent()
{
	// Game was deleted off the hard drive, let's remove it from the database
	if(pContentItem == NULL) 
		return S_FALSE;
	
	//Remove Game from current list
	if(pGameContent != NULL) {
		GameContentManager::getInstance().DeleteGameFromList(pGameContent);
		//Set focus up one if last item in list is removed
		if(listPack.CurrentIndex == (listPack.ListSize - 1)) {
			GameContentManager::getInstance().m_cGameList->SetCurSel(listPack.CurrentIndex - 1);
			GameContentManager::getInstance().m_cGameList->RefreshCurrentItem();
		} else {
			GameContentManager::getInstance().m_cGameList->SetCurSel(listPack.CurrentIndex);
			GameContentManager::getInstance().m_cGameList->RefreshCurrentItem();
		}
		m_Back.SetFocus(XUSER_INDEX_ANY);
		NavigateBack(XUSER_INDEX_ANY);
	}

	// Delete the item from the database
	ContentManager::getInstance().DeleteItem(pContentItem);

	return S_OK;
}

void CScnGameControls::UpdateFileProgress()
{
	ULONGLONG sizeDone = FileOperationManager::getInstance().GetSizeDone()/1024/1024;
	ULONGLONG totalSize = FileOperationManager::getInstance().GetTotalSize()/1024/1024;
	
	string currentStatus = FileOperationManager::getInstance().GetCurrentItemStatus();
	string PercentDone = sprintfaA("%d%%", (int)((sizeDone*1.0/totalSize*1.0)*100.0));

	m_FileName.SetText(strtowstr(currentStatus).c_str());
	m_OverallPercent.SetText(strtowstr(PercentDone).c_str());

	m_TotalProgressBar.SetValue((int)((sizeDone*1.0/totalSize*1.0)*100.0));
}

void CScnGameControls::ShowFileProgress(bool show)
{
	m_Achievements.SetShow(!show);
	m_GameScreenshots.SetShow(!show);
	m_GameSaves.SetShow(!show);
	m_RenameGame.SetShow(!show);
	m_DeleteGame.SetShow(!show);
	m_MoveGame.SetShow(!show);
	m_DownloadGame.SetShow(!show);
	m_ManageTUs.SetShow(!show);
	m_ManageTrainers.SetShow(!show);
	m_MoveXbox360.SetShow(!show);
	m_MoveXBLA.SetShow(!show);
	m_MoveHomebrew.SetShow(!show);
	m_MoveXbox1.SetShow(!show);
	m_MoveEmulators.SetShow(!show);
	m_MoveNoSection.SetShow(!show);

	m_Loading.SetShow(show);
	m_OverallPercent.SetShow(show);
	m_TotalProgressBar.SetShow(show);
	m_FileName.SetShow(show);
	m_GameTitle.SetShow(show);
}