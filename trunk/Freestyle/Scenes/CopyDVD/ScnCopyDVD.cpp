#include "stdafx.h"

#include "../../Tools/DVDInfo/DVDItem.h"
#include "../../Tools/DVDInfo/DVDWorker.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/Managers/Drives/DrivesManager.h"
#include "../FileBrowser/ScnFileBrowser.h"
#include "../../Application/FreestyleUIApp.h"

#include "ScnCopyDVD.h"
using namespace std;

CXuiScene sceneUser;
string fullcopy;

HRESULT CScnCopyDVD::OnInit(XUIMessageInit* pInitData, BOOL& bHandled)
{
    DebugMsg("CopyDVD","CopyDVD::OnInit");

	DVDMonitor::getInstance().add(*this);

	HRESULT hr = GetChildById( L"CopyDVD", &m_CopyDVD );
	isNested = hr == S_OK;

	DebugMsg("ScnCopyDVD","Scn is nested %d", isNested);

	if(isNested)
	{
		m_CopyDVD.GetChildById( L"FileName", &m_FileName );
		m_CopyDVD.GetChildById( L"TotalFileProgress", &m_TotalFileProgress );
		m_CopyDVD.GetChildById( L"TotalFileSizeProgress", &m_TotalFileSizeProgress );
		m_CopyDVD.GetChildById( L"TotalPercentComplete", &m_OverallPercent );
		m_CopyDVD.GetChildById( L"TotalProgressBar", &m_TotalProgressBar );

		m_CopyDVD.GetChildById( L"FileSizeComplete", &m_FileSizeComplete );
		m_CopyDVD.GetChildById( L"FileProgressBar", &m_FileProgressBar );
	    
		m_CopyDVD.GetChildById( L"OK", &m_OK );
		m_CopyDVD.GetChildById( L"Cancel", &m_Cancel );
		m_CopyDVD.GetChildById( L"CopyName", &m_CopyName );
		m_CopyDVD.GetChildById( L"CopyPath", &m_CopyPath );
		m_CopyDVD.GetChildById( L"IncludeUpdate", &m_IncludeUpdate );
		m_CopyDVD.GetChildById( L"PathText", &m_PathText );
		m_CopyDVD.GetChildById( L"NameText", &m_NameText );
		m_CopyDVD.GetChildById( L"border1", &m_border1 );
		m_CopyDVD.GetChildById( L"border2", &m_border2 );
		m_CopyDVD.GetChildById( L"Label_DestinationPath", &m_DestinationPath );
		m_CopyDVD.GetChildById( L"Label_TitleName", &m_TitleName );
		m_CopyDVD.GetChildById( L"Label_OverallTitle", &m_OverallTitle );
	}
	else
	{
		GetChildById( L"FileName", &m_FileName );
		GetChildById( L"TotalFileProgress", &m_TotalFileProgress );
		GetChildById( L"TotalFileSizeProgress", &m_TotalFileSizeProgress );
		GetChildById( L"TotalPercentComplete", &m_OverallPercent );
		GetChildById( L"TotalProgressBar", &m_TotalProgressBar );

		GetChildById( L"FileSizeComplete", &m_FileSizeComplete );
		GetChildById( L"FileProgressBar", &m_FileProgressBar );
	    
		GetChildById( L"OK", &m_OK );
		GetChildById( L"Cancel", &m_Cancel );
		GetChildById( L"CopyName", &m_CopyName );
		GetChildById( L"CopyPath", &m_CopyPath );
		GetChildById( L"IncludeUpdate", &m_IncludeUpdate );
		GetChildById( L"PathText", &m_PathText );
		GetChildById( L"NameText", &m_NameText );
		GetChildById( L"border1", &m_border1 );
		GetChildById( L"border2", &m_border2 );
		GetChildById( L"Label_DestinationPath", &m_DestinationPath );
		GetChildById( L"Label_TitleName", &m_TitleName );
		GetChildById( L"Label_OverallTitle", &m_OverallTitle );
	}

	GetChildById( L"Back", &m_back );
	Item = NULL;
	btnOk = new LPCWSTR[2]();
	btnOk[0] = L"OK";
	btnOk[1] = L"Cancel";

    m_TotalProgressBar.SetRange(0, 100);
	m_TotalProgressBar.SetValue(0);
	m_FileProgressBar.SetRange(0, 100);
	m_FileProgressBar.SetValue(0);

	m_NameText.SetText(L"Reading DVD");
	m_PathText.SetText(L"Reading DVD");

	// Disable buttons until title has been read
	m_Cancel.SetEnable(false);
	SetProgressShow(false);

	SetDVD();

    return S_OK;
}

HRESULT CScnCopyDVD::OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled)
{
    switch(pTimer->nId)
    {
        case TM_CopyDVD:
        {
			UpdateOverall();
            
			if(FileOperationManager::getInstance().isDone())
            {
                KillTimer(TM_CopyDVD);
				
                m_TotalProgressBar.SetValue(100);
				m_FileProgressBar.SetValue(100);
				m_FileSizeComplete.SetText(L" ");

				XamSetDvdSpindleSpeed(0);
				m_Cancel.SetShow(false);
				m_back.SetShow(true);

				// Added to help maintain focus after the operation is complete
				XuiElementSetFocus( m_back.m_hObj );
				// End of added focus control
				
				bHandled = TRUE;
			} else
			{
				UpdateFile();
			}

            break;
		}
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					m_NameText.SetText(buffer);
					copydvdname = wstrtostr(buffer);
					DebugMsg("CopyDVD", "Title name changed: %s", copydvdname.c_str());	
				}				
			}
			break;
		}
		case TM_FILE_MANAGER:
		{
			if(FileBrowserSelected == true)
            {
				if(FileBrowserFilename!="" )
                {
					copypath = FileBrowserFilename;
					m_PathText.SetText( strtowstr(FileBrowserFilename).c_str());
					if (Item->getFileType() == CONTENT_DVD_TYPE_XEX)
					{
						SETTINGS::getInstance().setDVDCopy360(copypath);
					} else if (Item->getFileType() == CONTENT_DVD_TYPE_XBE)
					{
						SETTINGS::getInstance().setDVDCopyOrig(copypath);
					}
					KillTimer(TM_FILE_MANAGER);
					bHandled = TRUE;
				}
			}
			break;
		}
    }
    return S_OK;
}

void CScnCopyDVD::SetDVD()
{
	CONTENT_DVD_TYPE type;
	string pathS;
	if (FileExistsA("dvd:\\default.xex") == 1)
	{
		type = CONTENT_DVD_TYPE_XEX;
		pathS = "games";
		ReadDVD(type, pathS);

	} else if (FileExistsA("dvd:\\default.xbe") == 1)
	{
		type = CONTENT_DVD_TYPE_XBE;
		pathS = "xbox1";
		ReadDVD(type, pathS);
	} else {

		ShowMessageBoxEx(L"XuiMessageBox2", this->m_hObj, L"No Game Disk Detected", L"Please insert Xbox 360 or Xbox Classic game disc and press OK.", 2, btnOk, 1, NULL, NULL);
	
		m_NameText.SetText(L"NO Disc");
		smc.OpenTray();
		m_OK.SetEnable(false);
	}
}

void CScnCopyDVD::ReadDVD(CONTENT_DVD_TYPE type, string pathS)
{
	m_NameText.SetText(L"Extracting");
	m_OK.SetEnable(true);
	DebugMsg("CopyDVD", "title: %s", copydvdname.c_str());
	copypath = SETTINGS::getInstance().getDVDCopy360();
	if (copypath == "")
	{
		if (FileExistsA("usb0:\\" + pathS))
		{
			copypath = sprintfaA("usb0:\\%s\\", pathS.c_str());
		} else if (FileExistsA("hdd1:\\" + pathS))
		{
			copypath = sprintfaA("hdd1:\\%s\\", pathS.c_str());
		} else {
			copypath = "can't set destination folder";
		}
	}
	m_PathText.SetText(strtowstr(copypath).c_str());
	Item = new DVDItem(type);
	Item->setRequester(this);
	DVDWorker::getInstance().StartItem(Item);
}

void CScnCopyDVD::ExtractCompleted(DVDItem* itm) {
	copydvdname = str_removeSpecialChar(itm->getTitle());
	m_NameText.SetText(strtowstr(copydvdname).c_str());
}

void CScnCopyDVD::UpdateFileCount(DVDItem* itm) {
	
	if (itm->getScanComplete())
	{
		SetTimer(TM_CopyDVD, 500);
		m_Cancel.SetEnable(true);
	} else {
		wstring Info = sprintfaW(L"Scanning File: %s", strtowstr(itm->getCurrentFile()).c_str());
		m_FileName.SetText(Info.c_str());
	}

}


HRESULT CScnCopyDVD::OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled)
{
    if ( hObjPressed == m_back )
    {
		XamSetDvdSpindleSpeed(0);
		FileBrowserSelectFolder = false;
		FileOperationManager::getInstance().EmptyQueue();
		if (Item != NULL)
		{
			Item->setRequester(NULL);
			delete Item;
			Item = NULL;
		}
		m_CopyPath.SetFocus(XUSER_INDEX_ANY);
		NavigateBack(XUSER_INDEX_ANY);
		bHandled = TRUE;

    } else if (hObjPressed == m_OK )
	{
		SetProgressShow(true);

		m_Cancel.SetFocus(XUSER_INDEX_ANY);

		m_TotalProgressBar.SetValue(0);
		fullcopy = copypath + copydvdname;
		if (!FileExists(fullcopy))
		{	
			_mkdir(fullcopy.c_str());
		}

		DebugMsg("CopyDVD", "Copy to: %s", fullcopy.c_str());
		m_TotalFileProgress.SetText(L"Preparing to Copy Files");
		bool syscheck;
		if (m_IncludeUpdate.IsChecked())
			syscheck = true;
		else
			syscheck = false;

		Item = new DVDItem(CONTENT_DVD_TYPE_UNKNOWN);
		Item->setRequester(this);
		Item->setScan(true, syscheck);
		Item->setDestPath(fullcopy);
		DVDWorker::getInstance().StartItem(Item);
		bHandled = TRUE;

	} else if (hObjPressed == m_Cancel)
	{
		XamSetDvdSpindleSpeed(0);

		FileOperationManager::getInstance().EmptyQueue();
		
		SetProgressShow(false);

		m_CopyName.SetFocus(XUSER_INDEX_ANY);

		bHandled = TRUE;

	} else if (hObjPressed == m_CopyPath)
	{
		FileBrowserSelectFolder = true;
        FileBrowserSelectFile = false;
		bHandled = TRUE;
		
		SkinManager::getInstance().setScene("filemanager.xur", *this, true);
		SetTimer(TM_FILE_MANAGER, 500);

	} else if (hObjPressed == m_CopyName)
	{
		bHandled = TRUE;
		buffer = new WCHAR[200];
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));
		DebugMsg("CopyDVD", "copydvdname: %s", copydvdname.c_str());
		dvdName = strtowstr(copydvdname);
		XShowKeyboardUI(0,VKBD_DEFAULT,dvdName.c_str(),L"Change folder name",L"Enter a new folder name",buffer,200,&keyboard);
		SetTimer(TM_KEYBOARD, 50);
	} 
    return S_OK;
}

HRESULT CScnCopyDVD::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{
	switch( pMsgBox->nButton )
    {
        case 0:
			smc.CloseTray();
            DebugMsg("CopyDVD", "OK Pressed");

			m_CopyPath.SetFocus(XUSER_INDEX_ANY);

            break;
        case 1:
            smc.CloseTray();
			DebugMsg("CopyDVD", "Cancel Pressed");
			m_NameText.SetText(L"Cancelling");

			NavigateBack(XUSER_INDEX_ANY);
            break;
    }
    
    bHandled = TRUE;

	return S_OK;
}


CScnCopyDVD::CScnCopyDVD()
{
	DebugMsg("CopyDVD", "Scn Created");
}

CScnCopyDVD::~CScnCopyDVD()
{
	DebugMsg("CopyDVD", "Scn Destroyed");
	DVDMonitor::getInstance().remove(*this);
	if (Item != NULL)
	{
		Item->setRequester(NULL);
		delete Item;
		Item = NULL;
	}
	VariablesCache::getInstance().setVariable("CDCOPY", "FALSE");
}

void CScnCopyDVD::UpdateOverall()
{
	ULONGLONG sizeDone = FileOperationManager::getInstance().GetSizeDone()/1024/1024;
	ULONGLONG totalSize = FileOperationManager::getInstance().GetTotalSize()/1024/1024;

	int nbFilesLeft = FileOperationManager::getInstance().GetFilesLeft();
	int nbFilesTotal = FileOperationManager::getInstance().GetTotalNumberOfFiles();
	int nbFileDone = nbFilesTotal - nbFilesLeft;

	string currentStatus = FileOperationManager::getInstance().GetCurrentItemStatus();
	string CurrentCount = sprintfaA("%d / %d File(s)", nbFileDone, nbFilesTotal );
	string CurrentSize = sprintfaA("%d mB / %d mB", sizeDone,totalSize );
	string PercentDone = sprintfaA("%d%%", (int)((sizeDone*1.0/totalSize*1.0)*100.0));

	m_FileName.SetText(strtowstr(currentStatus).c_str());
	m_TotalFileProgress.SetText(strtowstr(CurrentCount).c_str());
	m_TotalFileSizeProgress.SetText(strtowstr(CurrentSize).c_str());
	m_OverallPercent.SetText(strtowstr(PercentDone).c_str());
	
	m_TotalProgressBar.SetValue((int)((sizeDone*1.0/totalSize*1.0)*100.0));
}

void CScnCopyDVD::UpdateFile()
{
	ULONGLONG currentFileProgress = FileOperationManager::getInstance().GetCurrentFileProgress()/1024/1024;
	ULONGLONG currentFileSize = FileOperationManager::getInstance().GetItemSize()/1024/1024;

	string FileProgress = sprintfaA("%d mB / %d mB", currentFileProgress, currentFileSize );
	m_FileSizeComplete.SetText(strtowstr(FileProgress).c_str());

	m_FileProgressBar.SetValue(FileOperationManager::getInstance().GetCurrentProgressPercent());
}

void CScnCopyDVD::handle360GameDVDInsertedEvent()
{
	SetDVD();
}

void CScnCopyDVD::handleXbox1GameDVDInsertedEvent()
{
	SetDVD();
}

void CScnCopyDVD::handleTrayOpenEvent()
{
	m_NameText.SetText(L"Tray Open");
	copydvdname = "Tray Open";

	m_OK.SetEnable(false);
}

void CScnCopyDVD::handleTrayCloseEvent()
{
	m_NameText.SetText(L"Reading Disc");
	copydvdname = "Reading Disc";
}

void CScnCopyDVD::SetProgressShow(bool show)
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

	m_CopyPath.SetShow(!show);
	m_CopyName.SetShow(!show);
	m_OK.SetShow(!show);
	m_IncludeUpdate.SetShow(!show);
	m_PathText.SetShow(!show);
	m_NameText.SetShow(!show);
	m_back.SetShow(!show);
	m_border1.SetShow(!show);
	m_border2.SetShow(!show);
	m_DestinationPath.SetShow(!show);
	m_TitleName.SetShow(!show);
}