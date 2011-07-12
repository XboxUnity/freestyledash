//include <xmedia2.h>

#include "stdafx.h"

#include "ScnDualPane.h"
#include "../../Tools/Debug/Debug.h"
//#include "../../Tools/Managers/Drives/DrivesManager.h"
#include "../../Application/FreestyleApp.h"
#include "DestFileList/DestFileList.h"
#include "SourceFileList/SourceFileList.h"
#include "../FileBrowser/ScnFileBrowser.h"
#include "../../Tools/Managers/FileOperation/FileCallBack.h"
#include "../../Tools/Managers/FileOperation/FileCallBackItem.h"
#include "../../../Libs/libsmbd/xbox/xbLibSmb.h"
#include "../../Tools/Xex/Xbox360Container.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Managers/SambaClient/SambaClient.h"
#include "../../Scenes/GetCredentials/ScnGetCredentials.h"


#include "../../Tools/Xex/Xbox360Container.h"

using namespace std;

#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200


HRESULT CScnDualPane::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	keyboardMode = 0;

 	DebugMsg("ScnFileBrowser","CScnDualPane::OnInit");

	// Until we have a setting for this in the UI, uncommenting this one time is a simple way to turn on the samba client
	//SETTINGS::getInstance().setSambaClientOn(true);

	HRESULT hr = GetChildById( L"FileBrowserChildScn", &m_ChildScn );
	isNested = hr == S_OK;

	DebugMsg("ScnDualPane","Is Nested %d", isNested);

	if(isNested)
	{
		//Global
		m_ChildScn.GetChildById( L"TitleText", &m_title );
		m_ChildScn.GetChildById( L"Video", &m_video );

		//File Browser
		m_ChildScn.GetChildById( L"SrcFileList", &m_SrcFileList );
		m_ChildScn.GetChildById( L"DestFileList", &m_DestFileList );
		m_ChildScn.GetChildById( L"Launch", &m_Launch );
		m_ChildScn.GetChildById( L"Mark", &m_Mark );
		m_ChildScn.GetChildById( L"BtnCopy", &m_copy );
		m_ChildScn.GetChildById( L"BtnMove", &m_Move );
		m_ChildScn.GetChildById( L"BtnRen", &m_rename );
		m_ChildScn.GetChildById( L"BtnDel", &m_delete );
		m_ChildScn.GetChildById( L"BtnMkDir", &m_makedir );
		m_ChildScn.GetChildById( L"BtnLB", &m_LB );
		m_ChildScn.GetChildById( L"BtnRB", &m_RB );
		m_ChildScn.GetChildById( L"Background_Controls", &m_background_Controls );
		m_ChildScn.GetChildById( L"Background_SelectedFile", &m_background_SelectedFiles );
		m_ChildScn.GetChildById( L"Background_FilesSrc", &m_background_FilesSrc );
		m_ChildScn.GetChildById( L"Background_FilesDest", &m_background_FilesDest );
		m_ChildScn.GetChildById( L"Label_Right", &m_Right );
		m_ChildScn.GetChildById( L"Label_Left", &m_Left );
		m_ChildScn.GetChildById( L"Label_Actions", &m_LabelAction );

		//Progress Bar
		m_ChildScn.GetChildById( L"FileName", &m_FileName );
		m_ChildScn.GetChildById( L"TotalFileProgress", &m_TotalFileProgress );
		m_ChildScn.GetChildById( L"TotalFileSizeProgress", &m_TotalFileSizeProgress );
		m_ChildScn.GetChildById( L"TotalPercentComplete", &m_OverallPercent );
		m_ChildScn.GetChildById( L"TotalProgressBar", &m_TotalProgressBar );
		m_ChildScn.GetChildById( L"FileSizeComplete", &m_FileSizeComplete );
		m_ChildScn.GetChildById( L"FileProgressBar", &m_FileProgressBar );
		m_ChildScn.GetChildById( L"Label_OverallTitle", &m_OverallTitle );
		m_ChildScn.GetChildById( L"Cancel", &m_Cancel );
		m_ChildScn.GetChildById( L"img_LeftArrow", &m_ArrowLeft );
		m_ChildScn.GetChildById( L"img_LeftArrow1", &m_ArrowLeft1 );
		m_ChildScn.GetChildById( L"img_RightArrow", &m_ArrowRight );
		m_ChildScn.GetChildById( L"img_RightArrow1", &m_ArrowRight1 );

		//Prep Bar
		m_ChildScn.GetChildById( L"Prep", &m_Prep );
	}
	else
	{
		//Global
		GetChildById( L"TitleText", &m_title );
		GetChildById( L"Video", &m_video );

		//File Browser
		GetChildById( L"CopyText", &m_copytext );
		GetChildById( L"SrcFileList", &m_SrcFileList );
		GetChildById( L"DestFileList", &m_DestFileList );
		GetChildById( L"Launch", &m_Launch );
		GetChildById( L"Mark", &m_Mark );
		GetChildById( L"BtnCopy", &m_copy );
		GetChildById( L"BtnMove", &m_Move );
		GetChildById( L"BtnRen", &m_rename );
		GetChildById( L"BtnDel", &m_delete );
		GetChildById( L"BtnMkDir", &m_makedir );
		GetChildById( L"BtnLB", &m_LB );
		GetChildById( L"BtnRB", &m_RB );
		GetChildById( L"Background_Controls", &m_background_Controls );
		GetChildById( L"Background_SelectedFile", &m_background_SelectedFiles );
		GetChildById( L"Background_FilesSrc", &m_background_FilesSrc );
		GetChildById( L"Background_FilesDest", &m_background_FilesDest );
		GetChildById( L"Label_Right", &m_Right );
		GetChildById( L"Label_Left", &m_Left );
		GetChildById( L"Label_Actions", &m_LabelAction );
		GetChildById( L"img_LeftArrow", &m_ArrowLeft );
		GetChildById( L"img_LeftArrow1", &m_ArrowLeft1 );
		GetChildById( L"img_RightArrow", &m_ArrowRight );
		GetChildById( L"img_RighttArrow1", &m_ArrowRight1 );

		//Progress Bar
		GetChildById( L"FileName", &m_FileName );
		GetChildById( L"TotalFileProgress", &m_TotalFileProgress );
		GetChildById( L"TotalFileSizeProgress", &m_TotalFileSizeProgress );
		GetChildById( L"TotalPercentComplete", &m_OverallPercent );
		GetChildById( L"TotalProgressBar", &m_TotalProgressBar );
		GetChildById( L"FileSizeComplete", &m_FileSizeComplete );
		GetChildById( L"FileProgressBar", &m_FileProgressBar );
		GetChildById( L"Label_OverallTitle", &m_OverallTitle );
		GetChildById( L"Cancel", &m_Cancel );
		GetChildById( L"Prep", &m_Prep );
	}

	GetChildById( L"Back", &m_back );

	m_title.SetText(L"Starting");

	btnOk = new LPCWSTR[1]();
	btnOk[0] = L"OK";
	btnYesNo = new LPCWSTR[2]();
	btnYesNo[0] = L"Yes";
	btnYesNo[1] = L"No";

	xmvPlayer = NULL;
	selSrcItemCount = 0;
	selDestItemCount = 0;
	
	//Track wich message box is being pressed
	mb_Type = 0;

	CSrcFileList::m_FileBrowserSrc.UpToRoot();
	CDestFileList::m_FileBrowserDest.UpToRoot();

	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg,XM_FILES_REFRESH);
	XuiSendMessage( m_SrcFileList, &xuiMsg );
	XuiSendMessage( m_DestFileList, &xuiMsg );

	BOOL Temp = FALSE;
	OnDirChange(Temp);

	IsCut = false;
	Item = NULL;
	SetArrow(0);
	RBShift = false;
	LBShift = false;
	SetShift(false);
    SetPrepShow(false);
	SetProgressShow(false);
    return S_OK;
}

HRESULT CScnDualPane::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if (!copying) {
		if(pInputData->dwKeyCode == VK_PAD_RSHOULDER && !RBShift){
			RBShift = true;
			if (!LBShift)
			{
				SetShift(true);
				SetArrow(1);
			} else {

				SetShift(false);
				SetArrow(0);
			}
			bHandled = true;
		}else if(pInputData->dwKeyCode == VK_PAD_LSHOULDER && !LBShift){
			LBShift = true;
			if (!RBShift)
			{
				SetShift(true);
				SetArrow(2);
			} else {

				SetShift(false);
				SetArrow(0);
			}
			bHandled = true;
		}
	}
	return S_OK;
}

HRESULT CScnDualPane::OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if (!copying){
		if(pInputData->dwKeyCode == VK_PAD_RSHOULDER){
			RBShift = false;
			if (LBShift) {
				SetShift(true);
				SetArrow(2);
			} else {
				SetShift(false);
				SetArrow(0);
			}
			bHandled = true;
		}
		if(pInputData->dwKeyCode == VK_PAD_LSHOULDER){
			LBShift = false;
			if (RBShift) {
				SetShift(true);
				SetArrow(1);
			} else {
				SetShift(false);
				SetArrow(0);
			}
			bHandled = true;
		}
	}
	return S_OK;
}

HRESULT CScnDualPane::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (Shift)
	{
		if (hObjPressed == m_back) // Delete Function
		{
			if ((LBShift && !CSrcFileList::m_FileBrowserSrc.IsAtRoot()) || (RBShift && !CDestFileList::m_FileBrowserDest.IsAtRoot()))
			{
				IsCut = true;
				string warning;
				if ((LBShift && selSrcItemCount == 0) || (RBShift && selDestItemCount == 0))
				{
					FileInformation itm;
					if (LBShift) {
						itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
					} else if (RBShift) {
						itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
					}

					if (itm.path.substr(0,4).compare("smb:") == 0) {
						copysource = itm.path + "/" + itm.name;
						copysource = SambaClient::getInstance().RemovePassword(copysource);
					}
					else {
						copysource = itm.path + "\\" + itm.name;
					}
					warning = copysource;
					copydestination = "";
				
				} else {
					int itemCount = 0;
					int loopCount = 0;
					if (LBShift)
						loopCount = m_SrcFileList.GetItemCount();
					else if (RBShift)
						loopCount = m_DestFileList.GetItemCount();

					for (int x = 0; x < loopCount; x++)
					{
						FileInformation itm;
						if (LBShift) {
							itm = CSrcFileList::m_FileItemsSrc.at(x);
						} else if (RBShift) {
							itm = CDestFileList::m_FileItemsDest.at(x);
						}
						if (itm.isSelected)
						{
							if (itm.path.substr(0,4).compare("smb:") == 0) {
								copysource = itm.path + "/" + itm.name;
							}
							else {
								copysource = itm.path + "\\" + itm.name;
							}

							copysource = SambaClient::getInstance().RemovePassword(copysource);
							copydestination = "";
							if (itemCount == 0)
							{
								warning = copysource;
								itemCount++;
							} else {
								itemCount++;
								warning = warning + "\n" + copysource;
							}		
						}
					}
				}
				
				string message = sprintfaA("Are you sure you want to delete \n\n%s",warning.c_str());
				wcscpy_s(MessageText,1024,strtowstr(message).c_str());
				ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Delete file", MessageText, 2, btnYesNo, 1, NULL, NULL);
				mb_Type = 1;
			} else {
				XNotifyQueueUICustom(L"Can't Perform Function on Root Dir");
			}
			bHandled = TRUE;
		} else if (hObjPressed == m_Mark) // Move Funcion
		{
			IsCut = true;
			if (!CSrcFileList::m_FileBrowserSrc.IsAtRoot() && !CDestFileList::m_FileBrowserDest.IsAtRoot())
			{
				if ((RBShift && selSrcItemCount == 0) || (LBShift && selDestItemCount == 0))
				{
					FileInformation itm;
					string destination;
					if (RBShift) {
						itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
						destination = CDestFileList::m_FileBrowserDest.GetCurrentPath();
					} else if (LBShift) {
						destination = CSrcFileList::m_FileBrowserSrc.GetCurrentPath();
						itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
					}
					if (destination != "" && itm.name != "")
					{
						SetPrepShow(true);
						m_Cancel.SetFocus(XUSER_INDEX_ANY);
						if (itm.path.substr(0,4).compare("smb:") == 0) {
							copysource = itm.path + "/" + itm.name;
						}
						else {
							copysource = itm.path + "\\" + itm.name;
						}

						copysourcename = itm.name; 

						if (destination.substr(0,4).compare("smb:") == 0) {
							copydestination = destination + "/" + copysourcename;
						}
						else {
							copydestination = destination + "\\" + copysourcename;
						}
				
						Item = new FileCallBackItem();
						Item->setRequester(this);
						FileCallBack::getInstance().addSingleOperation(Item, itm, copydestination, IsCut);
					} else {
						XNotifyQueueUICustom(L"Please Select Destination To Move Files to");
					}
				} else 
				{
					string destination;
					if (RBShift)
						destination = CDestFileList::m_FileBrowserDest.GetCurrentPath();
					else if (LBShift)
						destination = CSrcFileList::m_FileBrowserSrc.GetCurrentPath();

					if (destination != "" && !CDestFileList::m_FileBrowserDest.IsAtRoot() && !CSrcFileList::m_FileBrowserSrc.IsAtRoot())
					{
						SetPrepShow(true);
						m_Cancel.SetFocus(XUSER_INDEX_ANY);
						Item = new FileCallBackItem();
						Item->setRequester(this);
						if (RBShift)
							FileCallBack::getInstance().addVectorOperation(Item, CSrcFileList::m_FileItemsSrc, destination, IsCut);
						else if (LBShift)
							FileCallBack::getInstance().addVectorOperation(Item, CDestFileList::m_FileItemsDest, destination, IsCut);
					} else {
						XNotifyQueueUICustom(L"Please Select Destination To Move Files to");
					}
				}

				selSrcItemCount = 0;
				selDestItemCount = 0;
			} else {
				XNotifyQueueUICustom(L"Cannot Move files to or From root.");
			}
			bHandled = TRUE;
		} else if (hObjPressed == m_rename)
		{

			DebugMsg("ScnDualPane","Selected Item Count %d", selSrcItemCount);
			if ((LBShift && !CSrcFileList::m_FileBrowserSrc.IsAtRoot()) || (RBShift && !CDestFileList::m_FileBrowserDest.IsAtRoot()))
			{
				if ((LBShift && selSrcItemCount == 0) || (RBShift && selDestItemCount == 0) && Shift)
				{
					FileInformation itm;
					if (LBShift) {
						itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
						leftRename = true;
					} else if (RBShift) {
						itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
						leftRename = false;
					}
					string source = itm.name;
					if (source.compare("..") != 0) {
						memset(result,0,sizeof(result));
						memset(&overlapped,0,sizeof(overlapped));
						wcscpy_s(DefaultName,1024,strtowstr(source).c_str());

						string message = sprintfaA("Please enter new name for file %s",source.c_str());
						wcscpy_s(MessageText,1024,strtowstr(message).c_str());
						XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,DefaultName,L"Please enter new name",MessageText,result,1024,&overlapped);
						SetTimer(TM_RENAME,50);
					}
				} else if ((LBShift && selSrcItemCount == 1) || (RBShift && selDestItemCount == 1) && Shift){
					for (int x = 0; x < m_SrcFileList.GetItemCount(); x++)
					{
						FileInformation itm;
						if (LBShift) {
							itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
							leftRename = true;
						} else if (RBShift) {
							itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
							leftRename = false;
						}
						if (itm.isSelected)
						{
							string source = itm.name;
							if (source.compare("..") == 0) continue;
							memset(result,0,sizeof(result));
							memset(&overlapped,0,sizeof(overlapped));
							wcscpy_s(DefaultName,1024,strtowstr(source).c_str());

							string message = sprintfaA("Please enter new name for file %s",source.c_str());
							wcscpy_s(MessageText,1024,strtowstr(message).c_str());
							XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,DefaultName,L"Please enter new name",MessageText,result,1024,&overlapped);

							SetTimer(TM_RENAME,50);
						}
					}
				} else {
					string message = sprintfaA("You can only rename 1 file at a time. Please highlight 1 file and try again");
					wcscpy_s(MessageText,1024,strtowstr(message).c_str());
					
					ShowMessageBoxEx(L"XuiMessageBox1", CFreestyleUIApp::getInstance().GetRootObj(), L"Rename File", MessageText, 1, btnOk, 0, NULL, NULL);
					mb_Type = 0;//Do Nothing with ok press
				}
			} else {
				XNotifyQueueUICustom(L"Cannot rename root folders");
			}
			bHandled = TRUE;
		} else if (hObjPressed == m_makedir)
		{
			string baseFile;
			int pathDepth;
			if (LBShift) {
				baseFile = CSrcFileList::m_FileBrowserSrc.GetCurrentPath();
				pathDepth = CSrcFileList::m_FileBrowserSrc.GetCurrentPathDepth();
				leftMakeDir = true;
			} else if (RBShift) {
				baseFile = CDestFileList::m_FileBrowserDest.GetCurrentPath();
				pathDepth = CDestFileList::m_FileBrowserDest.GetCurrentPathDepth();
				leftMakeDir = false;
			}
			if (baseFile.length() == 0 || (baseFile.substr(0,4).compare("smb:") == 0 && pathDepth < 4)) {
				// msg box about selecting a directory in the Destination pane

				XNotifyQueueUICustom(L"Please Select Appropriate location to create folder");
				ResetShift();
				bHandled = true;
				return S_OK;
			}
			memset(result,0,sizeof(result));
			memset(&overlapped,0,sizeof(overlapped));

			szTempString = sprintfaW(L"Please enter new directory name in %s",strtowstr(baseFile).c_str());
			//wcscpy_s(MessageText,1024,strtowstr(message).c_str());

			XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,L"New Folder",L"Please enter new directory name",szTempString.c_str(),result,1024,&overlapped);

			SetTimer(TM_KEYBOARD,50);

			bHandled = TRUE;
		}
	} else {  // Not shifted
		if (hObjPressed == m_back)
		{
			CXuiControl SrcCtrl;
			CXuiControl DestCtrl;

			m_SrcFileList.GetCurSel(&SrcCtrl);
			m_DestFileList.GetCurSel(&DestCtrl);

			if (SrcCtrl.HasFocus() && !CSrcFileList::m_FileBrowserSrc.IsAtRoot())
			{			
				XUIMessage xuiMsg;
				XuiMessage(&xuiMsg,XM_FILES_DOBACK);
				XuiSendMessage( m_SrcFileList, &xuiMsg );
			} else if (DestCtrl.HasFocus() && !CDestFileList::m_FileBrowserDest.IsAtRoot())
			{			
				XUIMessage xuiMsg;
				XuiMessage(&xuiMsg,XM_FILES_DOBACK);
				XuiSendMessage( m_DestFileList, &xuiMsg );
			} else
			{
				NavigateBack(XUSER_INDEX_ANY);
				DebugMsg("ScnDualPane","Navigate Back");
			}
			bHandled = TRUE;
		} 
		else if (hObjPressed == m_Mark) {
			if (m_SrcFileList.TreeHasFocus())
			{
				FileInformation itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
				DebugMsg("DualPane", "FileName : %s", itm.name.c_str());
				if (itm.isSelected)
				{
					DebugMsg("DualPane", "Deselecting");
					CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel()).isSelected = false;
					selSrcItemCount--;
				} else {
					DebugMsg("DualPane", "Selecting");
					CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel()).isSelected = true;
					selSrcItemCount++;
				}
			} else if (m_DestFileList.TreeHasFocus()) {
				FileInformation itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
				DebugMsg("DualPane", "FileName : %s", itm.name.c_str());
				if (itm.isSelected)
				{
					DebugMsg("DualPane", "Deselecting");
					CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel()).isSelected = false;
					selDestItemCount--;
				} else {
					DebugMsg("DualPane", "Selecting");
					CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel()).isSelected = true;
					selDestItemCount++;
				}
			}
			bHandled = TRUE;
		} 
		else if (hObjPressed == m_Cancel) {
			if (Item != NULL)
			{
				Item->setRequester(NULL);
				delete Item;
				Item = NULL;
			}
			FileBrowserSelectFolder = false;
			FileCallBack::getInstance().cancel();
			m_Prep.SetShow(false);
			SetProgressShow(false);
			m_Cancel.SetText(L"Cancel");
			XUIMessage xuiMsg;
			XuiMessage(&xuiMsg,XM_FILES_REFRESH);
			XuiSendMessage( m_DestFileList, &xuiMsg );
			XuiSendMessage( m_SrcFileList, &xuiMsg );
			m_SrcFileList.SetFocus(XUSER_INDEX_ANY);
			if(m_SrcFileList.GetCurSel() == -1)
			{
				m_SrcFileList.SetCurSel(0);
			}
			ResetShift();
			bHandled = TRUE;
		}
	}
	return S_OK;
} 

HRESULT CScnDualPane::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{
	switch( pMsgBox->nButton )
    {
        case 0:
			if(mb_Type == 1)
			{
				if ((LBShift && selSrcItemCount == 0) || (RBShift && selDestItemCount == 0))
				{
					DebugMsg("Deleting %s",copysource.c_str());
					SetPrepShow(true);
					m_Cancel.SetFocus(XUSER_INDEX_ANY);
					
					FileInformation itm;
					if (LBShift) {
						itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
					} else if (RBShift) {
						itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
					}

					Item = new FileCallBackItem();
					Item->setRequester(this);
					FileCallBack::getInstance().addSingleOperation(Item, itm, "", true);
				} else {
					SetPrepShow(true);
					m_Cancel.SetFocus(XUSER_INDEX_ANY);
					Item = new FileCallBackItem();
					Item->setRequester(this);
					if (LBShift) {
						FileCallBack::getInstance().addVectorOperation(Item, CSrcFileList::m_FileItemsSrc, "", true);
					} else if (RBShift) {
						FileCallBack::getInstance().addVectorOperation(Item, CDestFileList::m_FileItemsDest, "", true);
					}
				}
				
				selSrcItemCount = 0;
				selDestItemCount = 0;
				mb_Type = 0;
			}
            break;
			
    }
	ResetShift();
    bHandled = TRUE;

	return S_OK;
}

CScnDualPane::~CScnDualPane()
{
	FileOperationManager::getInstance().EmptyQueue();
	copysource = "";
	copydestination = "";
	IsCut = false;
	
	if (xmvPlayer != NULL)
	{
		xmvPlayer->Stop(XMEDIA_STOP_IMMEDIATE);
		xmvPlayer->Release();
		xmvPlayer = NULL;
	}
}
void CScnDualPane::UpdateInfo(FileCallBackItem *Itm) 
{
	if (Itm->getFirstAfterPrep()) {
		SetPrepShow(false);
		SetProgressShow(true);
		m_Cancel.SetFocus(XUSER_INDEX_ANY);
		Itm->setFirstAfterPrep(false);
	}
	UpdateOverall(Itm);
    
	if(Itm->getIsDone())
    {
        m_TotalProgressBar.SetValue(100);
		m_FileProgressBar.SetValue(100);
		m_FileSizeComplete.SetText(L" ");
		m_Cancel.SetText(L" OK");

	} else
	{
		UpdateFile(Itm);
	}
}

HRESULT CScnDualPane::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId)
	{
	case TM_KEYBOARD:
		{
			if (XHasOverlappedIoCompleted(&overlapped))
			{
				if (overlapped.dwExtendedError == ERROR_SUCCESS)
				{
					string newname = wstrtostr(result);
					FileInformation ditm;
					if (leftMakeDir)
						ditm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
					else
						ditm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
					string dest;

					if (ditm.path.substr(0,4).compare("smb:") == 0) {
						dest = ditm.path + "/" + newname;
						DebugMsg("FileBrowser", "Making dir %s",dest.c_str());
						smbc_mkdir(dest.c_str(),0666);
					}
					else {
						dest = ditm.path + "\\" + newname;
						DebugMsg("FileBrowser", "Making dir %s",dest.c_str());
						_mkdir(dest.c_str());
					}

					//string dest = ditm.path + "\\"  + newname;
								

					XUIMessage xuiMsg;
					XuiMessage(&xuiMsg,XM_FILES_REFRESH);
					XuiSendMessage( m_SrcFileList, &xuiMsg );
					XuiSendMessage( m_DestFileList, &xuiMsg );

					//DebugMsg("Rename %s to %s",source.c_str(),dest.c_str());
				}
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
			}
			break;
		}
	case TM_RENAME:
		{
			if (XHasOverlappedIoCompleted(&overlapped))
			{
				if (overlapped.dwExtendedError == ERROR_SUCCESS)
				{
					string newname = wstrtostr(result);
					FileInformation itm;
					if (leftRename) {
						itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
					} else {
						itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
					}
					
					string source;
					
					if (itm.path.substr(0,4).compare("smb:") == 0) {
						source = itm.path + "/" + itm.name;
					}
					else {
						source = itm.path + "\\" + itm.name;
					}

					//= itm.path + "\\" + itm.name;

					string dest;
					if (itm.path.substr(0,4).compare("smb:") == 0) {
						dest = itm.path + "/" + newname;
					}
					else {
						dest = itm.path + "\\" + newname;
					}
										
					//= itm.path + "\\" + newname;
					if (itm.path.substr(0,4).compare("smb:") == 0) {
						smbc_rename(source.c_str(), dest.c_str());
					}
					else {
						rename(source.c_str(), dest.c_str());
					}

					XUIMessage xuiMsg;
					XuiMessage(&xuiMsg,XM_FILES_REFRESH);
					XuiSendMessage( m_SrcFileList, &xuiMsg );
					XuiSendMessage( m_DestFileList, &xuiMsg );

					DebugMsg("FileBrowser", "Rename %s to %s",source.c_str(),dest.c_str());
				}
				KillTimer(TM_RENAME);
				bHandled = TRUE;
			}
			break;
		}

	}
	return S_OK;
}

HRESULT CScnDualPane::OnFilesChange( BOOL& bHandled )
{
	bHandled = TRUE;
	return S_OK;
}

HRESULT CScnDualPane::OnDirChange( BOOL& bHandled )
{
	if (Shift) {
		Copy();
	} else {
		CXuiControl SrcCtrl;
		CXuiControl DestCtrl;

		m_SrcFileList.GetCurSel(&SrcCtrl);
		m_DestFileList.GetCurSel(&DestCtrl);
		string text;
		if (SrcCtrl.HasFocus()) {
			text = CSrcFileList::m_FileBrowserSrc.GetCurrentPath();
			text = SambaClient::getInstance().RemovePassword(text);
			if (CSrcFileList::m_FileBrowserSrc.IsAtRoot())
				text = "Root";
			m_Left.SetText(strtowstr(text).c_str());
		} else if (DestCtrl.HasFocus()) {
			text = CDestFileList::m_FileBrowserDest.GetCurrentPath();
			text = SambaClient::getInstance().RemovePassword(text);
			if (CDestFileList::m_FileBrowserDest.IsAtRoot())
				text = "Root";
			m_Right.SetText(strtowstr(text).c_str());
		}

		/*if (FL_CurrentDir.empty())
			text.append("\\");*/
		//m_title.SetText(strtowstr(text).c_str());
	}
	bHandled = TRUE;
	return S_OK;
}

HRESULT CScnDualPane::OnSmbPermissionDenied( Credentials* cred, BOOL& bHandled )
{
	//
	//GetCredentials(L"XuiMessageBox2", this, L"Get Credentials", MessageText, 2, btnYesNo, 1, NULL, NULL);

	GetCredentials(cred->smbPath, cred->user, cred->password, cred->share);
	delete cred;
	bHandled = true;
	return S_OK;
}


HRESULT CScnDualPane::OnFilePress( BOOL& bHandled )
{
	if (Shift) {
		Copy();
	} else {
		FileInformation itm;
		if (m_SrcFileList.TreeHasFocus())
		{
			itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
		} else if (m_DestFileList.TreeHasFocus())
		{
			itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
		}

		string file = itm.path + "\\" + itm.name;

		string text = "Pressed " + file;
		m_copytext.SetText(strtowstr(text).c_str());

		string ext = make_lowercaseA(FileExtA(itm.name));

		if (ext == "xex")
		{
			XLaunchNewImage(file.c_str(),0);
		} else if (ext == "xbe")
		{
			// Attempting to launch XBE files
			XLaunchNewImage(file.c_str(),0);
		} else if (ext == "wmv")
		{
			if (xmvPlayer==NULL)
			{
				XMEDIA_XMV_CREATE_PARAMETERS parameters;
				ZeroMemory(&parameters, sizeof (parameters));
				IXAudio2* pXAudio2 = NULL;
				IXAudio2MasteringVoice* pMasterVoice = NULL;
		
				xmvPlayer = NULL;
				XAudio2Create( &pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ;
				pXAudio2->CreateMasteringVoice( &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL ) ;

				//Set Up Default Parameters
				parameters.createType = XMEDIA_CREATE_FROM_FILE;
				parameters.dwAudioStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
				parameters.dwVideoStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
				parameters.createFromFile.dwIoBlockSize = (DWORD)512000;
				parameters.createFromFile.dwIoBlockCount = (DWORD)10;
				parameters.createFromFile.dwIoBlockJitter = (DWORD)10;
				parameters.createFromFile.szFileName = file.c_str();

				XMedia2CreateXmvPlayer(CFreestyleApp::getInstance().m_pd3dDevice,pXAudio2,&parameters,&xmvPlayer);
			
				m_video.SetPlayer(xmvPlayer);
			} else {
				xmvPlayer->Stop(XMEDIA_STOP_IMMEDIATE);
				xmvPlayer->Release();
				xmvPlayer = NULL;
			}
		} else {
			Xbox360Container conFile;
			HRESULT retVal = conFile.OpenContainer(file.c_str());
			if(retVal == S_OK)
			{
				conFile.Mount("filemgr");
				conFile.LaunchGame();
				conFile.CloseContainer();
			}
		}
	}
	bHandled = TRUE;
	return S_OK;
}

void CScnDualPane::UpdateOverall(FileCallBackItem* Itm)
{
	ULONGLONG sizeDone = Itm->getSizeDone();
	ULONGLONG totalSize = Itm->getSizeTotal();

	int nbFilesTotal = Itm->getTotalFiles();
	int nbFileDone = Itm->getFilesDone();

	string currentStatus = Itm->getCurrent();
	string CurrentCount = sprintfaA("%d / %d File(s)", nbFileDone, nbFilesTotal );
	string CurrentSize = sprintfaA("%d mB / %d mB", sizeDone,totalSize );
	string PercentDone = sprintfaA("%d%%", Itm->getOverallPercent());

	string passwordsRemoved = SambaClient::getInstance().RemovePassword(currentStatus);
	m_FileName.SetText(strtowstr(passwordsRemoved).c_str());
	m_TotalFileProgress.SetText(strtowstr(CurrentCount).c_str());
	m_TotalFileSizeProgress.SetText(strtowstr(CurrentSize).c_str());
	m_OverallPercent.SetText(strtowstr(PercentDone).c_str());
	
	m_TotalProgressBar.SetValue(Itm->getOverallPercent());
}

void CScnDualPane::UpdateFile(FileCallBackItem* Itm)
{
	ULONGLONG currentFileProgress = Itm->getCurrentSizeComplete();
	ULONGLONG currentFileSize = Itm->getCurrentSize();

	string FileProgress = sprintfaA("%d mB / %d mB", currentFileProgress, currentFileSize );
	m_FileSizeComplete.SetText(strtowstr(FileProgress).c_str());

	m_FileProgressBar.SetValue(Itm->getCurrentPercent());
}

void CScnDualPane::SetProgressShow(bool show)
{

	m_FileName.SetShow(show);
	m_TotalFileProgress.SetShow(show);
	m_TotalProgressBar.SetShow(show);
	m_OverallPercent.SetShow(show);
	m_TotalFileSizeProgress.SetShow(show);
	m_Cancel.SetShow(show);
	m_FileSizeComplete.SetShow(show);
	m_FileProgressBar.SetShow(show);
	m_OverallTitle.SetShow(show);

	m_copytext.SetShow(!show);
	m_SrcFileList.SetShow(!show);
	m_DestFileList.SetShow(!show);
	m_copy.SetShow(!show);
	m_Move.SetShow(!show);
	m_Mark.SetShow(!show);
	m_LB.SetShow(!show);
	m_RB.SetShow(!show);
	m_rename.SetShow(!show);
	m_delete.SetShow(!show);
	m_makedir.SetShow(!show);
	m_background_Controls.SetShow(!show);
	m_background_SelectedFiles.SetShow(!show);
	m_background_FilesSrc.SetShow(!show);
	m_background_FilesDest.SetShow(!show);
	m_back.SetShow(!show);
	m_back.SetEnable(!show);
	m_Right.SetShow(!show);
	m_Left.SetShow(!show);
	m_LabelAction.SetShow(!show);

	if (show)
	{
		SetShift(false);
		SetArrow(0);
	}
	copying = show;

}

void CScnDualPane::SetPrepShow(bool show)
{
	if (show) {
		SetShift(false);
		SetArrow(0);
	}
	copying = true;

	m_Prep.SetShow(show);
	m_Cancel.SetShow(show);
	
	m_FileName.SetShow(!show);
	m_TotalFileProgress.SetShow(!show);
	m_TotalProgressBar.SetShow(!show);
	m_OverallPercent.SetShow(!show);
	m_TotalFileSizeProgress.SetShow(!show);
	
	m_FileSizeComplete.SetShow(!show);
	m_FileProgressBar.SetShow(!show);
	m_OverallTitle.SetShow(!show);

	m_copytext.SetShow(!show);
	m_SrcFileList.SetShow(!show);
	m_DestFileList.SetShow(!show);
	m_copy.SetShow(!show);
	m_Move.SetShow(!show);
	m_Mark.SetShow(!show);
	m_rename.SetShow(!show);
	m_delete.SetShow(!show);
	m_makedir.SetShow(!show);
	m_Launch.SetShow(!show);
	m_RB.SetShow(!show);
	m_LB.SetShow(!show);
	m_background_Controls.SetShow(!show);
	m_background_SelectedFiles.SetShow(!show);
	m_background_FilesSrc.SetShow(!show);
	m_background_FilesDest.SetShow(!show);
	m_back.SetShow(!show);
	m_back.SetEnable(!show);
	m_Right.SetShow(!show);
	m_Left.SetShow(!show);
	m_LabelAction.SetShow(!show);

}

void CScnDualPane::SetShift(bool show)
{
	Shift = show;
	CSrcFileList::Shift = show;
	CDestFileList::Shift = show;
	m_makedir.SetEnable(show);
	m_rename.SetEnable(show);

}

void CScnDualPane::SetArrow(int arr)
{
	bool arrleft = false;
	bool arrright = false;
	if (arr == 1)
		arrright = true;
	else if (arr ==2)
		arrleft = true;
	m_ArrowLeft.SetShow(arrleft);
	m_ArrowLeft1.SetShow(arrleft);
	m_ArrowRight.SetShow(arrright);
	m_ArrowRight1.SetShow(arrright);
}

void CScnDualPane::Copy()
{
	IsCut = false;
	bool left = false;
	if (m_SrcFileList.TreeHasFocus())
		left = true;

	if (!CSrcFileList::m_FileBrowserSrc.IsAtRoot() && !CDestFileList::m_FileBrowserDest.IsAtRoot())
	{
		if ((RBShift && selSrcItemCount == 0) || (LBShift && selDestItemCount == 0))
		{
			FileInformation itm;
			string destination;
			if (RBShift) {
				itm = CSrcFileList::m_FileItemsSrc.at(m_SrcFileList.GetCurSel());
				destination = CDestFileList::m_FileBrowserDest.GetCurrentPath();
			} else if (LBShift) {
				destination = CSrcFileList::m_FileBrowserSrc.GetCurrentPath();
				itm = CDestFileList::m_FileItemsDest.at(m_DestFileList.GetCurSel());
			}

			if (destination != "" && itm.name != "")
			{
				SetPrepShow(true);
				m_Cancel.SetFocus(XUSER_INDEX_ANY);
				if (itm.path.substr(0,4).compare("smb:") == 0) {
					copysource = itm.path + "/" + itm.name;
				}
				else {
					copysource = itm.path + "\\" + itm.name;
				}
				copysourcename = itm.name; 				

				if (destination.substr(0,4).compare("smb:") == 0) {
					copydestination = destination + "/" + copysourcename;
				}
				else {
					copydestination = destination + "\\" + copysourcename;
				}

				Item = new FileCallBackItem();
				Item->setRequester(this);
				FileCallBack::getInstance().addSingleOperation(Item, itm, copydestination, IsCut);
			} else {
				XNotifyQueueUICustom(L"Please Select Destination To copy Files to");
			}
		} else 
		{
			string destination;
			if (RBShift)
				destination = CDestFileList::m_FileBrowserDest.GetCurrentPath();
			else 
				destination = CSrcFileList::m_FileBrowserSrc.GetCurrentPath();
			if (destination != "" && !CDestFileList::m_FileBrowserDest.IsAtRoot() && !CSrcFileList::m_FileBrowserSrc.IsAtRoot())
			{
				SetPrepShow(true);
				m_Cancel.SetFocus(XUSER_INDEX_ANY);
				Item = new FileCallBackItem();
				Item->setRequester(this);
				if (RBShift)
					FileCallBack::getInstance().addVectorOperation(Item, CSrcFileList::m_FileItemsSrc, destination, IsCut);
				else
					FileCallBack::getInstance().addVectorOperation(Item, CDestFileList::m_FileItemsDest, destination, IsCut);
			} else {
				XNotifyQueueUICustom(L"Please Select Destination To copy Files to");
			}
		}
		selSrcItemCount = 0;
		selDestItemCount = 0;
	} else {
		XNotifyQueueUICustom(L"Cannot Copy to Or from Root Folders");
	}
}

void CScnDualPane::ResetShift()
{
	RBShift = false;
	LBShift = false;

	for (DWORD i=0; i< 3; i++ )
	{

		DWORD dwResult ;
		XINPUT_STATE pState;
		ZeroMemory( &pState, sizeof(XINPUT_STATE) );

		dwResult  = XInputGetState(i, &pState);
		if (dwResult == ERROR_SUCCESS)
		{
			DebugMsg("DualPane", "Buttons: %04X", pState.Gamepad.wButtons);
			if (pState.Gamepad.wButtons == 0x0100)
			{
				LBShift = true;	
			} else if (pState.Gamepad.wButtons == 0x0200) {
				RBShift = true;
			}
		}
	}

	if (LBShift) {
		if (!RBShift) {
			SetShift(true);
			SetArrow(2);
		} else {
			SetShift(false);
			SetArrow(0);
		}
	} else if (RBShift) {
		if (!LBShift) {
			SetShift(true);
			SetArrow(1);
		} else {
			SetShift(false);
			SetArrow(0);
		}
	} else {
		SetShift(false);
		SetArrow(0);
	}
}