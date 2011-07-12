#include "stdafx.h"

#include "ScnManageTUs.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Debug/Debug.h"
#include "ActiveList/ActiveList.h"
#include "PathList/TUPaths.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../stdafx.h"

#include "../../Application/FreestyleUIApp.h"
#include "../../Tools/XEX/Xbox360Container.h"
#include "../../Tools/GameContent/GameContentManager.h"
#include "../../Application/Freestyleapp.h"
#include "../../Tools/Managers/TitleUpdates/TUDownloadManager.h"
#include "../../Tools/Managers/TitleUpdates/TitleUpdateManager.h"

using namespace std;



HRESULT CScnManageTUs::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnManageTUs","CScnManageTUs::OnInit");
	HRESULT HR = GetChildById( L"TitleManagerScene", &m_TitleManager);
	isNested = HR == S_OK;
	if (isNested) {
		m_TitleManager.GetChildById( L"ActiveTUList", &m_ActiveList );
		m_TitleManager.GetChildById( L"DownloadTU", &m_Download );
		m_TitleManager.GetChildById( L"Delete", &m_DeleteUpdate );
		m_TitleManager.GetChildById( L"Upload", &m_Upload );
		m_TitleManager.GetChildById( L"TUPaths", &m_TUPaths );
		m_TitleManager.GetChildById( L"Label_ActiveTU", &m_ActiveLabel );
		m_TitleManager.GetChildById( L"Label_TUPaths", &m_PathLabel );
		m_TitleManager.GetChildById( L"Background_ActiveTU", &m_ActiveBackground );
	} else {
		GetChildById( L"ActiveTUList", &m_ActiveList );
		GetChildById( L"DownloadTU", &m_Download );
		GetChildById( L"Delete", &m_DeleteUpdate );
		GetChildById( L"Upload", &m_Upload );
		GetChildById( L"TUPaths", &m_TUPaths );
		GetChildById( L"Label_ActiveTU", &m_ActiveLabel );
		GetChildById( L"Label_TUPaths", &m_PathLabel );
		GetChildById( L"Background_ActiveTU", &m_ActiveBackground );
	}
	GetChildById( L"Back", &m_Back );
	
	showPaths(false);

	TUDownloadManager::getInstance().setRequester(this);
	
	btnOk = new LPCWSTR[2]();
	btnOk[0] = L"OK";
	btnOk[1] = L"Cancel";

	return S_OK;
}

void CScnManageTUs::showPaths(bool show)
{
	m_TUPaths.SetShow(show);
	m_TUPaths.SetEnable(show);
	m_PathLabel.SetShow(show);
	m_Download.SetEnable(!show);
	m_DeleteUpdate.SetEnable(!show);
	m_Upload.SetEnable(!show);
	m_ActiveList.SetShow(!show);
	m_ActiveList.SetEnable(!show);
	m_ActiveBackground.SetShow(!show);
	m_ActiveLabel.SetShow(!show);
	if (show)
	{
		m_TUPaths.SetFocus(XUSER_INDEX_ANY);
	} else {
		m_ActiveList.SetFocus(XUSER_INDEX_ANY);
	}
}
HRESULT CScnManageTUs::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	GameListContentPtr vListContent = GameContentManager::getInstance().getCurrentListContent();

	// Display Background Texture
	if(pGetSourceImageData->iData == FSDIMAGE_GAMEBACKGROUND && !pGetSourceImageData->bItemData){
		pGetSourceImageData->hBrush = GameContentManager::getInstance().getSourceImageTexture(FSDIMAGE_GAMEBACKGROUND, NULL,  vListContent );
		bHandled = true;
	}

	return S_OK;
}

HRESULT CScnManageTUs::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_Upload)
	{
		if(CFreestyleApp::getInstance().hasInternetConnection() == false) {
			XNotifyQueueUICustom(L"No network connection detected.");
		} else {
			XNotifyQueueUICustom(L"Transferring Title Updates");
			TUDownloadManager::getInstance().addTUDownload(false, -1);
		}
		bHandled = TRUE;

	} else if (hObjPressed == m_Back)
	{
		bHandled  = true;
		if (m_TUPaths.IsShown())
		{
			showPaths(false);
			CTUPathList::TUPaths.clear();
			m_TUPaths.DeleteItems(0, m_TUPaths.GetItemCount());
		} else {
			TUDownloadManager::getInstance().removeRequester();
			NavigateBack(XUSER_INDEX_ANY);
		}
	} else if (hObjPressed == m_ActiveList)
	{
		int selected = m_ActiveList.GetCurSel();
		if (selected != -1)
		{
			SQLTitleUpdate* Tu = CActiveList::ActiveTU.at(selected);
			GAMECONTENT_LIST_STATE listPack;
			listPack = GameContentManager::getInstance().getGameListSnapshot();
			DWORD TitleID = listPack.CurrentGame->ContentRef->getTitleId();
			vector<SQLTitleUpdate*> tuVec = FSDSql::getInstance().getTitleUpdatesForTitleId(0, TitleID);
			if (!Tu->isDisabled)
			{
				for (unsigned int x = 0; x < tuVec.size(); x++)
				{
					if (tuVec.at(x)->titleUpdateId == Tu->isBackupFor ||
						tuVec.at(x)->isBackupFor == Tu->titleUpdateId )
					{
						FSDSql::getInstance().updateTitleDisabled(0, tuVec.at(x)->titleUpdateId, true);
					}
				}
				FSDSql::getInstance().updateTitleDisabled(0, Tu->titleUpdateId, true);
			} else {
				if (Tu->filename.find("tu") != string::npos)
				{
					for (unsigned int x = 0; x < tuVec.size(); x++)
					{
						FSDSql::getInstance().updateTitleDisabled(0, tuVec.at(x)->titleUpdateId, true);
					}
				}
				for (unsigned int x = 0; x < tuVec.size(); x++)
				{
					if (tuVec.at(x)->titleUpdateId == Tu->isBackupFor ||
						tuVec.at(x)->isBackupFor == Tu->titleUpdateId )
					{
						FSDSql::getInstance().updateTitleDisabled(0, tuVec.at(x)->titleUpdateId, false);
					}
				}
				FSDSql::getInstance().updateTitleDisabled(0, Tu->titleUpdateId, false);
			}
			
			XUIMessage xuiMsg;
			XuiMessage(&xuiMsg,XM_REFRESH);
			XuiSendMessage( m_ActiveList, &xuiMsg );
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_Download)
	{		
		if(CFreestyleApp::getInstance().hasInternetConnection() == false) {
			XNotifyQueueUICustom(L"No network connection detected.");
		} else {
			CTUPathList::TUPaths.clear();
			CTUPathList::TUPaths = FSDSql::getInstance().getTitleUpdatePaths();
			if (CTUPathList::TUPaths.size() > 1)
			{
				m_TUPaths.InsertItems(0, CTUPathList::TUPaths.size());
				showPaths(true);
			} else {
				ULONGLONG pathID = CTUPathList::TUPaths.at(0)->pathId;
				TUDownloadManager::getInstance().addTUDownload(true, pathID);
			}
		}
		bHandled = TRUE;	
	} else if (hObjPressed == m_DeleteUpdate)
	{
		if (CActiveList::ActiveTU.size() > 0)
		{
			ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Delete Update", L"Are you sure you want to delete this update?", 2, btnOk, 1, NULL, NULL);
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_TUPaths)
	{
		int pathNum = m_TUPaths.GetCurSel();
		ULONGLONG pathID = CTUPathList::TUPaths.at(pathNum)->pathId;
		CTUPathList::TUPaths.clear();
		m_TUPaths.DeleteItems(0, m_TUPaths.GetItemCount());
		showPaths(false);
		TUDownloadManager::getInstance().addTUDownload(true, pathID);
		bHandled = true;
	}

	return S_OK;
}  

HRESULT CScnManageTUs::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{

	switch( pMsgBox->nButton )
	{
		case 0:
			{
				//Remove from ActiveList
				int selected = m_ActiveList.GetCurSel();
				if (selected != -1)
				{
					GAMECONTENT_LIST_STATE listPack;
					listPack = GameContentManager::getInstance().getGameListSnapshot();
					DWORD titleID = listPack.CurrentGame->ContentRef->getTitleId();
					SQLTitleUpdate* tu = CActiveList::ActiveTU.at(selected);
					TitleUpdateManager::getInstance().DeleteTitleUpdates(titleID, tu);
					while (TitleUpdateManager::getInstance().isWorking())
					{
						Sleep(50);
					}
					
					int count = m_ActiveList.GetItemCount();
					if (selected >= count)
					{
						m_ActiveList.SetCurSel(selected -1);
					}
					XUIMessage xuiMsg;
					XuiMessage(&xuiMsg,XM_REFRESH);
					XuiSendMessage( m_ActiveList, &xuiMsg );
				}
			}
			break;            
	}
    
    bHandled = TRUE;

	return S_OK;
}

void CScnManageTUs::done()
{
	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg,XM_REFRESH);
	XuiSendMessage( m_ActiveList, &xuiMsg );
}