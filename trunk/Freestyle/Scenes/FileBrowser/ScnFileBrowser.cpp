//include <xmedia2.h>

#include "stdafx.h"

#include "ScnFileBrowser.h"
//#include "scnutilities.h"
#include "scnfileop.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Managers/Drives/DrivesManager.h"
#include "../../Application/FreestyleApp.h"
#include "FilesList/FilesList.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"

using namespace std;

bool FileBrowserSelectFile = false;
bool FileBrowserSelectFolder = false;
bool FileBrowserSelected = false;
string FileBrowserFilename;
IXMedia2XmvPlayer* xmvPlayer = NULL;


HRESULT CScnFileBrowser::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
 	DebugMsg("ScnFileBrowser","CScnFileBrowser::OnInit");
	HRESULT hr = GetChildById( L"FileBrowserChildScn", &m_ChildScn );
	isNested = hr == S_OK;

	DebugMsg("ScnDualPane","Is Nested %d", isNested);

	if(isNested)
	{
		//Global
		m_ChildScn.GetChildById( L"TitleText", &m_title );
		m_ChildScn.GetChildById( L"Video", &m_video );

		//File Browser
		m_ChildScn.GetChildById( L"CopyText", &m_copytext );
		m_ChildScn.GetChildById( L"FileList", &m_filelist );
		m_ChildScn.GetChildById( L"Copy", &m_copy );
		m_ChildScn.GetChildById( L"Cut", &m_cut );
		m_ChildScn.GetChildById( L"Paste", &m_paste );
		m_ChildScn.GetChildById( L"Rename", &m_rename );
		m_ChildScn.GetChildById( L"Delete", &m_delete );
		m_ChildScn.GetChildById( L"MakeDir", &m_makedir );
		m_ChildScn.GetChildById( L"Background_Controls", &m_background_Controls );
		m_ChildScn.GetChildById( L"Background_SelectedFile", &m_background_SelectedFiles );
		m_ChildScn.GetChildById( L"Background_Files", &m_background_Files );

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
	}
	else
	{
		//Global
		GetChildById( L"TitleText", &m_title );
		GetChildById( L"Video", &m_video );

		//File Browser
		GetChildById( L"CopyText", &m_copytext );
		GetChildById( L"FileList", &m_filelist );
		GetChildById( L"Copy", &m_copy );
		GetChildById( L"Cut", &m_cut );
		GetChildById( L"Paste", &m_paste );
		GetChildById( L"Rename", &m_rename );
		GetChildById( L"Delete", &m_delete );
		GetChildById( L"MakeDir", &m_makedir );
		GetChildById( L"Background_Controls", &m_background_Controls );
		GetChildById( L"Background_SelectedFile", &m_background_SelectedFiles );
		GetChildById( L"Background_Files", &m_background_Files );


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
	}


	GetChildById( L"Back", &m_Back );
	GetChildById( L"SelectDir", &m_dir );

	m_title.SetText(L"Starting");

	btnOk = new LPCWSTR[1]();
	btnOk[0] = L"OK";
	btnYesNo = new LPCWSTR[2]();
	btnYesNo[0] = L"Yes";
	btnYesNo[1] = L"No";
	

	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg,XM_FILES_REFRESH);
	XuiSendMessage( m_filelist, &xuiMsg );

	BOOL Temp = FALSE;
	OnDirChange(Temp);
	m_paste.SetEnable(false);

	IsCut = false;
    FileBrowserSelected = false;

	m_dir.SetShow(false);

    if(FileBrowserSelectFile)
    {
        m_copytext.SetText(L"Please choose a file");
        m_dir.SetShow(false);
		m_dir.SetEnable(false);
    }
    else if(FileBrowserSelectFolder)
    {
        m_copytext.SetText(L"Please choose a directory");
        m_dir.SetShow(true);
		m_dir.SetEnable(true);
    }
    else
    {
        m_copytext.SetText(L"No copied files");
        m_dir.SetShow(false);
		m_dir.SetEnable(false);
    }
	SetProgressShow(false);
    return S_OK;
}

WCHAR DefaultName[1024];
WCHAR MessageText[1024];
string CScnFileBrowser::GetSelectedFile()
{
		string baseFile = CFilesList::m_FileBrowser.GetCurrentPath() ;
		baseFile = baseFile + "\\";
		FileItem itm = CFilesList::m_FileItems.at(m_filelist.GetCurSel());
		baseFile = baseFile + itm.name;
		return baseFile;
}
string CScnFileBrowser::GetSelectedFileName()
{
		FileItem itm = CFilesList::m_FileItems.at(m_filelist.GetCurSel());
		return itm.name;
}
HRESULT CScnFileBrowser::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_Back)
	{
		if (!CFilesList::m_FileBrowser.IsAtRoot())
		{
			XUIMessage xuiMsg;
			XuiMessage(&xuiMsg,XM_FILES_DOBACK);
			XuiSendMessage( m_filelist, &xuiMsg );
		}
        else
        {
            FileBrowserSelected = true;
            FileBrowserFilename = "";

			NavigateBack(XUSER_INDEX_ANY);
        }
		bHandled = TRUE;
	} 
    else if (hObjPressed == m_dir)
	{
        string baseFile = CFilesList::m_FileBrowser.GetCurrentPath();
	    baseFile = baseFile + "\\";

        FileBrowserSelected = true;
        FileBrowserFilename = baseFile;

        NavigateBack(XUSER_INDEX_FOCUS);
		bHandled = TRUE;

	} else if (hObjPressed == m_delete)
	{
		IsCut = true;
		
		copysource = GetSelectedFile();
		deleteSelection = GetCurSel();
		copydestination = "";

		string message = sprintfaA("Are you sure you want to delete %s",copysource.c_str());
		wcscpy_s(MessageText,1024,strtowstr(message).c_str());

		ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Delete file", MessageText, 2, btnYesNo, 1, NULL, NULL);

		bHandled = TRUE;

	} else if (hObjPressed == m_copy)
	{
		IsCut = false;
		
		copysource = GetSelectedFile();
		copysourcename = GetSelectedFileName(); 
	
		string text = "Copy : " + copysource;
		m_copytext.SetText(strtowstr(text).c_str());
		m_paste.SetEnable(true);
		bHandled = TRUE;

	} else if (hObjPressed == m_cut)
	{
		IsCut = true;
		
		copysource = GetSelectedFile();
		copysourcename = GetSelectedFileName(); 
		m_paste.SetEnable(true);
		string text = "Cut : " + copysource;
		m_copytext.SetText(strtowstr(text).c_str());
		bHandled = TRUE;
	} else if (hObjPressed == m_paste)
	{
		copydestination = CFilesList::m_FileBrowser.GetCurrentPath() + "\\" + copysourcename;
		
		DebugMsg("ScnFileBrowser","Copy %s to %s",copysource.c_str(),copydestination.c_str());

		m_paste.SetEnable(false);
		if (IsFolder(copysource))
		{
			FileOperationManager::getInstance().AddFolderOperation(copysource, copydestination, IsCut);
		} else {
			FileOperationManager::getInstance().AddFileOperation(copysource, copydestination, IsCut);
		}
		FileOperationManager::getInstance().DoWork();
		SetProgressShow(true);
		m_Cancel.SetFocus(XUSER_INDEX_ANY);
		SetTimer(TM_FILE_MANAGER, 50);

		bHandled = TRUE;
	}  else if (hObjPressed == m_rename)
	{
		string source = GetSelectedFileName();

		memset(result,0,sizeof(result));
		memset(&overlapped,0,sizeof(overlapped));
		wcscpy_s(DefaultName,1024,strtowstr(source).c_str());

		string message = sprintfaA("Please enter new name for file %s",source.c_str());
		wcscpy_s(MessageText,1024,strtowstr(message).c_str());
		XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,DefaultName,L"Please enter new name",MessageText,result,1024,&overlapped);

		SetTimer(TM_RENAME,50);

		bHandled = TRUE;
	} else if (hObjPressed == m_makedir)
	{
		IsCut = true;

		string baseFile = CFilesList::m_FileBrowser.GetCurrentPath();

		memset(result,0,sizeof(result));
		memset(&overlapped,0,sizeof(overlapped));

		string message = sprintfaA("Please enter new directory name in %s",baseFile.c_str());
		wcscpy_s(MessageText,1024,strtowstr(message).c_str());

		XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,L"New Folder",L"Please enter new directory name",MessageText,result,1024,&overlapped);

		SetTimer(TM_KEYBOARD,50);

		bHandled = TRUE;
	} else if (hObjPressed == m_exitscene)
	{
		KillTimer(TM_FILE_MANAGER);
		FileBrowserSelectFolder = false;
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

		NavigateBack(XUSER_INDEX_ANY);
		bHandled = TRUE;
	} else if (hObjPressed == m_Cancel)
	{
		KillTimer(TM_FILE_MANAGER);
		FileBrowserSelectFolder = false;
		FileOperationManager::getInstance().EmptyQueue();

		SetProgressShow(false);
		XUIMessage xuiMsg;
		XuiMessage(&xuiMsg,XM_FILES_REFRESH);
		XuiSendMessage( m_filelist, &xuiMsg );
		m_filelist.SetFocus(XUSER_INDEX_ANY);
		m_filelist.SetCurSel(1);

		bHandled = TRUE;
	}
	return S_OK;
} 

HRESULT CScnFileBrowser::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{

	switch( pMsgBox->nButton )
    {
        case 0:
			DebugMsg("Deleting %s",copysource.c_str());

			if (IsFolder(copysource))
			{
				FileOperationManager::getInstance().AddFolderOperation(copysource, copydestination, IsCut);
			} else {
				FileOperationManager::getInstance().AddFileOperation(copysource, copydestination, IsCut);
			}
			FileOperationManager::getInstance().DoWork();
			SetProgressShow(true);
			SetTimer(TM_FILE_MANAGER, 50);
            break;
    }
    
    bHandled = TRUE;

	return S_OK;
}

HRESULT CScnFileBrowser::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId)
	{
	case TM_FILE_MANAGER:
		{
			UpdateOverall();
            
			if(FileOperationManager::getInstance().isDone())
            {
                m_TotalProgressBar.SetValue(100);
				m_FileProgressBar.SetValue(100);
				m_FileSizeComplete.SetText(L" ");

				SetProgressShow(false);
				FileOperationManager::getInstance().Terminate();

				IsCut = false;
				m_filelist.SetFocus(XUSER_INDEX_ANY);

				if(deleteSelection == m_filelist.GetItemCount() - 1)
					m_filelist.SetCurSel(deleteSelection-1);
				else
					m_filelist.SetCurSel(deleteSelection);

				XUIMessage xuiMsg;
				XuiMessage(&xuiMsg,XM_FILES_REFRESH);
				XuiSendMessage( m_filelist, &xuiMsg );
				KillTimer(TM_FILE_MANAGER);

				bHandled = TRUE;
			} else
			{
				UpdateFile();
			}

            break;
		}

	case TM_KEYBOARD:
		{
			if (XHasOverlappedIoCompleted(&overlapped))
			{
				if (overlapped.dwExtendedError == ERROR_SUCCESS)
				{
					string newname = wstrtostr(result);
					string dest = CFilesList::m_FileBrowser.GetCurrentPath()+ "\\"  + newname;
				
					DebugMsg("FileBrowser", "Making dir %s",dest.c_str());
					_mkdir(dest.c_str());

					XUIMessage xuiMsg;
					XuiMessage(&xuiMsg,XM_FILES_REFRESH);
					XuiSendMessage( m_filelist, &xuiMsg );

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
					string source = GetSelectedFile();

					string dest = CFilesList::m_FileBrowser.GetCurrentPath()+ "\\" + newname;
						
					rename(source.c_str(), dest.c_str());

					XUIMessage xuiMsg;
					XuiMessage(&xuiMsg,XM_FILES_REFRESH);
					XuiSendMessage( m_filelist, &xuiMsg );

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

HRESULT CScnFileBrowser::OnFilesChange( BOOL& bHandled )
{
	return S_OK;
}

HRESULT CScnFileBrowser::OnDirChange( BOOL& bHandled )
{
	string text = CFilesList::m_FileBrowser.GetCurrentPath();
	/*if (FL_CurrentDir.empty())
		text.append("\\");*/
	m_title.SetText(strtowstr(text).c_str());
	return S_OK;
}

HRESULT CScnFileBrowser::OnFilePress( BOOL& bHandled )
{
	string file = GetSelectedFile();

	string text = "Pressed " + file;
	m_copytext.SetText(strtowstr(text).c_str());

	string ext = make_lowercaseA(FileExtA(file));

    if(FileBrowserSelectFolder)
    {
        // Do nothing
		//NavigateBack(XUSER_INDEX_FOCUS);
        return S_OK;
    }

    FileBrowserSelected = true;

    if(FileBrowserSelectFile)
    {
        FileBrowserFilename = file;
        //NavigateBack(XUSER_INDEX_FOCUS);
        return S_OK;
    }

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

			XMedia2CreateXmvPlayer(CFreestyleApp::m_pd3dDevice,pXAudio2,&parameters,&xmvPlayer);
		
			m_video.SetPlayer(xmvPlayer);
		} else {
			xmvPlayer->Stop(XMEDIA_STOP_IMMEDIATE);
			xmvPlayer->Release();
			xmvPlayer = NULL;
		}
	}
	return S_OK;
}

void CScnFileBrowser::UpdateOverall()
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

void CScnFileBrowser::UpdateFile()
{
	ULONGLONG currentFileProgress = 0;
	if(FileOperationManager::getInstance().GetCurrentFileProgress()!= 0)
	{
		currentFileProgress = FileOperationManager::getInstance().GetCurrentFileProgress()/1024/1024;
	}
	ULONGLONG currentFileSize = FileOperationManager::getInstance().GetItemSize()/1024/1024;

	string FileProgress = sprintfaA("%d mB / %d mB", currentFileProgress, currentFileSize );
	m_FileSizeComplete.SetText(strtowstr(FileProgress).c_str());

	m_FileProgressBar.SetValue(FileOperationManager::getInstance().GetCurrentProgressPercent());
}

void CScnFileBrowser::SetProgressShow(bool show)
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
	m_filelist.SetShow(!show);
	m_copy.SetShow(!show);
	m_cut.SetShow(!show);
	m_paste.SetShow(!show);
	m_rename.SetShow(!show);
	m_delete.SetShow(!show);
	m_makedir.SetShow(!show);
	m_background_Controls.SetShow(!show);
	m_background_SelectedFiles.SetShow(!show);
	m_background_Files.SetShow(!show);
	m_Back.SetShow(!show);
	m_exitscene.SetShow(!show);
	m_exitscene.SetEnable(!show);

}