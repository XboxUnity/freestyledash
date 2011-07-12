//include <xmedia2.h>

#include "stdafx.h"

#include "ScnManagePaths.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../FileBrowser/ScnFileBrowser.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/Drives/DrivesManager.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/ContentList/ContentManager.h"
#include "RemovePathCallBack.h"

#include "PathList/PathList.h"

using namespace std;


HRESULT CScnManagePaths::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnManagePaths","CScnManagePaths::OnInit");
	HRESULT HR = GetChildById( L"PathManagerScene", &m_ManagePaths);
	isNested = HR == S_OK;

	if (isNested)
	{
		m_ManagePaths.GetChildById( L"PathList", &m_PathList );
		m_ManagePaths.GetChildById( L"Retail", &m_Retail );
		m_ManagePaths.GetChildById( L"Devkit", &m_Devkit );
		m_ManagePaths.GetChildById( L"PathText", &m_PathText );
		m_ManagePaths.GetChildById( L"Label_Devkit", &m_Label_Devkit );
		m_ManagePaths.GetChildById( L"ScanDepthText", &m_ScanDepthText );
		m_ManagePaths.GetChildById( L"SetScanDepth", &m_ScanDepth );
		m_ManagePaths.GetChildById( L"ScanDepth", &m_LabelScanDepth );
		m_ManagePaths.GetChildById( L"PickPath", &m_PickPath );
		m_ManagePaths.GetChildById( L"Label_Retail", &m_Label_Retail );
		m_ManagePaths.GetChildById( L"Label_PathList", &m_Label_PathList );
		m_ManagePaths.GetChildById( L"Add", &m_Add );
		m_ManagePaths.GetChildById( L"Remove", &m_Delete );
		m_ManagePaths.GetChildById( L"Save", &m_Save );
		m_ManagePaths.GetChildById( L"Edit", &m_Edit );
	} else {
		GetChildById( L"PathList", &m_PathList );
		GetChildById( L"Retail", &m_Retail );
		GetChildById( L"Devkit", &m_Devkit );
		GetChildById( L"PathText", &m_PathText );
		GetChildById( L"Label_Devkit", &m_Label_Devkit );
		GetChildById( L"ScanDepth", &m_LabelScanDepth );
		GetChildById( L"ScanDepthText", &m_ScanDepthText );
		GetChildById( L"SetScanDepth", &m_ScanDepth );
		GetChildById( L"PickPath", &m_PickPath );
		GetChildById( L"Label_Retail", &m_Label_Retail );
		GetChildById( L"Label_PathList", &m_Label_PathList );
		GetChildById( L"Add", &m_Add );
		GetChildById( L"Remove", &m_Delete );
		GetChildById( L"Save", &m_Save );
		GetChildById( L"Edit", &m_Edit );
	}

	GetChildById( L"Back", &m_BackButton );

	int count = m_Retail.GetItemCount();
	m_Retail.DeleteItems(0, count);
	count = m_Devkit.GetItemCount();
	m_Devkit.DeleteItems(0, count);
	m_ScanDepth.SetValue(2);
	vector<SQLTab*> Tabs = FSDSql::getInstance().getTabs();
	m_Retail.InsertItems(0, Tabs.size());
	m_Devkit.InsertItems(0, Tabs.size());
	for (int x = 0; x < (int)Tabs.size(); x++)
	{
		int item = (int)Tabs.at(x)->tabId;
		m_Retail.SetText(item, strtowstr(Tabs.at(x)->tabName).c_str());
		m_Devkit.SetText(item, strtowstr(Tabs.at(x)->tabName).c_str());
	}
	// Setup our Buffers and Variables for Messagebox UI
	m_msgButtons = new LPCWSTR[2];
	m_msgButtons[0] = L"OK"; m_msgButtons[1] = L"Cancel";

	Itm = -1;
	HideList(false);

	//Hide action buttons untill scn recieves focus
	HideButtons();

	//Bool to detect if we're in the edit/add section
	managePath = false;

	return S_OK;
}

HRESULT CScnManagePaths::OnSetFocus(HXUIOBJ hObjLosingFocus, BOOL& bHandled)
{
	if(XuiElementTreeHasFocus(this->m_hObj) == TRUE)
		ShowButtons();

	return S_OK;
}


HRESULT CScnManagePaths::OnNotifyKillFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
	HXUIOBJ parentLoosing, listParent, checkBoxListParent;
	XuiElementGetParent(pNotifyFocusData->hObjOther , &parentLoosing);
	XuiElementGetParent(parentLoosing, &listParent);
	XuiElementGetParent(listParent, &checkBoxListParent);

	if(parentLoosing == this->m_hObj)//Single parent items (buttons, sliders, parent->item)
	{
		ShowButtons();
	}
	else if(listParent == this->m_hObj)//Two deep (parent->List->control_listItem)
	{
		ShowButtons();
	}
	else if(checkBoxListParent == this->m_hObj)//Three deep (parent->List->ListItemGroup->controlListItem)
	{
		ShowButtons();
	}
	else
	{
		HideButtons();
	}

	return S_OK;
}

HRESULT CScnManagePaths::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_PathList)
	{
		bHandled = TRUE;
		Itm = m_PathList.GetCurSel();
		m_PathText.SetText(strtowstr(CPathList::ScanPaths.at(Itm).Path).c_str());
		if (CPathList::ScanPaths.at(Itm).ScanDepth == -1)
		{
			m_ScanDepthText.SetText(L"Infinite");
		} else {
			wstring depth = sprintfaW(L"%d",CPathList::ScanPaths.at(Itm).ScanDepth);
			m_ScanDepthText.SetText(depth.c_str());
		}
		m_ScanDepth.SetValue(CPathList::ScanPaths.at(Itm).ScanDepth);
		int RType = (int)CPathList::ScanPaths.at(Itm).RetailTabId;
		int DType = (int)CPathList::ScanPaths.at(Itm).DevkitTabId;
		for (int x = 0; x < m_Retail.GetItemCount(); x++)
		{
			if (x == RType)
				m_Retail.SetItemCheck(x, true);
			else
				m_Retail.SetItemCheck(x, false);
			if (x == DType)
				m_Devkit.SetItemCheck(x, true);
			else 
				m_Devkit.SetItemCheck(x, false);
		}			

		HideList(true);
		
	} else if (hObjPressed == m_Add)
	{
		
		m_PathText.SetText(L"Location to add to Path List");
		for (int x = 0; x < m_Retail.GetItemCount(); x++)
		{
			m_Retail.SetItemCheck(x, false);
			m_Devkit.SetItemCheck(x, false);
		}
		m_Retail.SetItemCheck(0, true);
		m_Devkit.SetItemCheck(0, true);
		int iDepth;
		m_ScanDepth.GetValue(&iDepth);
		wstring depth = sprintfaW(L"%d",iDepth);
		m_ScanDepthText.SetText(depth.c_str());
		HideList(true);

		bHandled = TRUE;

	}  else if (hObjPressed == m_Save)
	{
		if (managePath)
		{
			ScanPath TempItm;
			if (Itm != -1)
				TempItm = CPathList::ScanPaths.at(Itm);
			TempItm.Path = wstrtostr(m_PathText.GetText());
			if (strcmp(TempItm.Path.c_str(), "Location to add to Path List")!=0)
			{	
				TempItm.DevkitTabId = 0;
				TempItm.RetailTabId = 0;
				for (int x = 0; x < m_Retail.GetItemCount(); x++)
				{
					if (m_Devkit.GetItemCheck(x))
					{
						TempItm.DevkitTabId = x;
					}
					if (m_Retail.GetItemCheck(x))
					{
						TempItm.RetailTabId = x;
					}
				}
				m_ScanDepth.GetValue(&TempItm.ScanDepth);
				if (Itm == -1)
				{
					string root = TempItm.Path.substr(0, TempItm.Path.find_first_of(":")+1);
					string path = TempItm.Path.substr(TempItm.Path.find_first_of(":")+1);

					Drive* d = DrivesManager::getInstance().getDriveByMountPoint(root);
					if (d != NULL) {
						string devId = d->getSerialStr();
						TempItm.PathId = FSDSql::getInstance().addScanPath(path, devId, TempItm.RetailTabId, TempItm.DevkitTabId, TempItm.ScanDepth);
						if (TempItm.PathId == -1) {
							ScanPath* existingPath = FSDSql::getInstance().getScanPath(path, devId);
							// that path exists already, so have to call modifyScanPath...but that means deleting all content for that scanpath 
							// (because it might be from a deeper depth, for example)
							// and then we should go off and scan for content again...				
							delete existingPath;
						} else {
							if(SETTINGS::getInstance().getDisableAutoScan() == FALSE) {
								ContentManager::getInstance().AddScanPath(TempItm);
							}
						}
					}

				} else {
					string root = TempItm.Path.substr(0, TempItm.Path.find_first_of(":")+1);
					string path = TempItm.Path.substr(TempItm.Path.find_first_of(":")+1);

					Drive* d = DrivesManager::getInstance().getDriveByMountPoint(root);
					if (d != NULL) {
						string devId = d->getSerialStr();
						FSDSql::getInstance().updateScanPath(path, devId, TempItm.PathId, TempItm.RetailTabId, TempItm.DevkitTabId, TempItm.ScanDepth);
						if(SETTINGS::getInstance().getDisableAutoScan() == FALSE) {
							ContentManager::getInstance().AddScanPath(TempItm);
						}
					}
					Itm = -1;
				}
				HideList(false);
				XUIMessage xuiMsg;
				XuiMessage(&xuiMsg,XM_REFRESH);
				XuiSendMessage( m_PathList, &xuiMsg );
				m_PathList.SetFocus();
			}
		} else {
			if (m_PathList.GetItemCount() > 0)
			{
				wstring m_szMsgboxWarningCaption = L"Warning";
				wstring m_szMsgboxClearDataPrompt = L"This will remove all content in the path from the Database. Are you sure you want to continue?";
				if (HTTPDownloader::getInstance().getStatus() == "")
				{					
					ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(),m_szMsgboxWarningCaption.c_str(), m_szMsgboxClearDataPrompt.c_str(), 2, m_msgButtons, 1, NULL, NULL);
				} else {
					XNotifyQueueUICustom(L"Please wait for Download to complete");	
				}
			}
		}
		bHandled = TRUE;
			
	} else if (hObjPressed == m_PickPath)
	{
		FileBrowserSelectFolder = true;
        FileBrowserSelectFile = false;
		bHandled = TRUE;
		
		SkinManager::getInstance().setScene("filemanager.xur", *this, true);
		SetTimer(TM_FILE_MANAGER, 500);
	} else if (hObjPressed == m_Retail)
	{
		int Item = m_Retail.GetCurSel();
		BOOL checked = m_Retail.GetItemCheck(Item);
		for (int x = 0; x < m_Retail.GetItemCount(); x++)
		{
			if (x == Item)
				m_Retail.SetItemCheck(x, checked);
			else 
				m_Retail.SetItemCheck(x, false);
		}
		if (m_Retail.GetCheckedItemCount() == 0)
			m_Retail.SetItemCheck(0, true);
		bHandled = TRUE;
	} else if (hObjPressed == m_Devkit)
	{
		int Item = m_Devkit.GetCurSel();
		BOOL checked = m_Devkit.GetItemCheck(Item);
		for (int x = 0; x < m_Devkit.GetItemCount(); x++)
		{
			if (x == Item)
				m_Devkit.SetItemCheck(x, checked);
			else 
				m_Devkit.SetItemCheck(x, false);
		}
		if (m_Devkit.GetCheckedItemCount() == 0)
			m_Devkit.SetItemCheck(0, true);
		bHandled = TRUE;
	} else if (hObjPressed == m_BackButton)
	{
		if (managePath)
		{
			HideList(false);
			Itm = -1;
		} else {
			NavigateBack(XUSER_INDEX_ANY);
		}
		bHandled = TRUE;
	}

	return S_OK;
}


HRESULT CScnManagePaths::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled )
{
	wstring sScanDepth = L"";
	if (pNotifyValueChangedData->nValue != -1)
	{
		sScanDepth = sprintfaW(L"%d", pNotifyValueChangedData->nValue);
	} else {
		sScanDepth = sprintfaW(L"Infinite");\
	}
	m_ScanDepthText.SetText(sScanDepth.c_str());
	bHandled = TRUE;
	return S_OK;
}
HRESULT CScnManagePaths::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId)
	{
	case TM_FILE_MANAGER:
		{
			if(FileBrowserSelected == true)
            {
				if(FileBrowserFilename!="" )
                {
					m_PathText.SetText( strtowstr(FileBrowserFilename).c_str());
					KillTimer(TM_FILE_MANAGER);
					bHandled = TRUE;
				}
			}
			break;
		}
	}
	return S_OK;
}

HRESULT CScnManagePaths::OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled)
{
	switch( pXUIMessageMessageBoxReturn->nButton )
    {
	case 0:
		{		
			m_WaitInfo.title = L"Deleting Game Data";
			m_WaitInfo.type = 2;
			m_WaitInfo.reboot = false;

			ScanPath tempPath = CPathList::ScanPaths.at(m_PathList.GetCurSel());
			RemovePathCallBack::getInstance().removePath(tempPath);

			SkinManager::getInstance().setScene("wait.xur", *this, true, "ScnWait", &m_WaitInfo);
			XUIMessage xuiMsg;
			XuiMessage(&xuiMsg,XM_REFRESH);
			XuiSendMessage( m_PathList, &xuiMsg );
			m_PathList.SetFocus();
			m_PathList.SetCurSel(1);
			break;
		}
	default:
		break;
	};

	// Return Successfully
	bHandled = TRUE;
	return S_OK;
}

void CScnManagePaths::HideList(bool hide)
{
	m_PathText.SetShow(hide);
	m_Label_Devkit.SetShow(hide);
	m_ScanDepthText.SetShow(hide);
	m_LabelScanDepth.SetShow(hide);
	m_Devkit.SetShow(hide);
	m_ScanDepth.SetShow(hide);
	m_Retail.SetShow(hide);
	m_PickPath.SetShow(hide);
	m_Label_Retail.SetShow(hide);

	managePath = hide;

	m_Label_PathList.SetShow(!hide);
	m_PathList.SetShow(!hide);
	m_Add.SetShow(!hide);
	m_Add.SetEnable(!hide);

	if (hide)
	{
		m_Save.SetText(L"Save");
		m_PickPath.SetFocus(XUSER_INDEX_ANY);
	} else {
		m_Save.SetText(L"Delete");
		m_PathList.SetFocus(XUSER_INDEX_ANY);
		m_PathList.SetCurSel(0);
	}
}

void CScnManagePaths::ShowButtons()
{
	if(!managePath)
		m_Add.SetShow(true);
	else
		m_Add.SetShow(false);

	m_Save.SetShow(true);
	m_Delete.SetShow(true);
	m_Edit.SetShow(true);
}

void CScnManagePaths::HideButtons()
{
	m_Add.SetShow(false);
	m_Save.SetShow(false);
	m_Delete.SetShow(false);
	m_Edit.SetShow(false);
}

void CScnManagePaths::AddPath()
{
	//This is called from the settings scn and will check if any paths exists
	// If they do just give it foucs. If not then open the add path scn
	if(m_PathList.GetItemCount() == 0)
	{
		m_PathText.SetText(L"Location to add to Path List");
		for (int x = 0; x < m_Retail.GetItemCount(); x++)
		{
			m_Retail.SetItemCheck(x, false);
			m_Devkit.SetItemCheck(x, false);
		}
		m_Retail.SetItemCheck(0, true);
		m_Devkit.SetItemCheck(0, true);
		int iDepth;
		m_ScanDepth.GetValue(&iDepth);
		wstring depth = sprintfaW(L"%d",iDepth);
		m_ScanDepthText.SetText(depth.c_str());
		HideList(true);
	}

}

int CScnManagePaths::HandleBack()
{
	if (managePath)
	{
		HideList(false);
		Itm = -1;
		return 1; //Dont Nav Back
	} else {
		return 0; //Nav Back
	}
}
