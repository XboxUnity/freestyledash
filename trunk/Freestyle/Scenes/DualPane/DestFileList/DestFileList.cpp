#include "stdafx.h"

#include "DestFileList.h"

#include <algorithm>
#include <sys/stat.h>
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/FileBrowser/FileBrowser.h"
#include "../../../Tools/Managers/SambaClient/SambaClient.h"
#include "../../../../Libs/libsmbd/xbox/xbLibSmb.h"
#include "../../../Scenes/GetCredentials/ScnGetCredentials.h"
using namespace std;

vector<FileInformation> CDestFileList::m_FileItemsDest;
FileBrowser CDestFileList::m_FileBrowserDest;
bool CDestFileList::Shift;
bool CDestFileList::bIgnoreStoredCredentials;

bool DestSortFiles(const FileInformation  left, const FileInformation  right) 
{ 
	if(left.isDir && left.name == "..")
	{
		return true;
	}
	if(right.isDir && right.name == "..")
	{
		return false;
	}
	if (left.isDir != right.isDir) return left.isDir;

	int res = _stricmp(left.name.c_str(),right.name.c_str());
	if (res < 0) return true;
	return false;
}

HRESULT CDestFileList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("DestFilesList","CDestFileList::OnInit");
	LoadSettings("DestFilesList", *this);

	// Load Settings
	ListIconPaths.szStandardFileIcon = strtowstr(GetSetting("STANDARDFILEPATH", ""));
	ListIconPaths.szStandardFolderIcon = strtowstr(GetSetting("STANDARDFOLDERPATH", ""));
	ListIconPaths.szSelectedFileIcon = strtowstr(GetSetting("SELECTEDFILEPATH", ""));
	ListIconPaths.szSelectedFolderIcon = strtowstr(GetSetting("SELECTEDFOLDERPATH", ""));

	CXuiElement parent;
	this->GetParent(&parent);
	parent.GetParent(&parent);
	m_FileBrowserDest.setParentScene(parent.m_hObj);
	bIgnoreStoredCredentials = false;

	return S_OK;
}

HRESULT CDestFileList::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if(pInputData->dwKeyCode == VK_PAD_BACK){
		bIgnoreStoredCredentials = true;
		bHandled = false; // want this to be passed up the chain
	}
	return S_OK;
}

HRESULT CDestFileList::OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if(pInputData->dwKeyCode == VK_PAD_BACK){
		bIgnoreStoredCredentials = false;
		bHandled = false; // want this to be passed up the chain
	}
	return S_OK;
}


HRESULT CDestFileList::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	return S_OK;
}
    

// Gets called every frame
HRESULT CDestFileList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	//DebugMsg("OnGetSourceDataText - %d",pGetSourceTextData->iData);
	// filename
	if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {
		
		pGetSourceTextData->szText = m_FileItemsDest.at(pGetSourceTextData->iItem).wname.c_str();
		bHandled = TRUE;
	}
	// extra text
	if( ( 1 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {
		pGetSourceTextData->szText = m_FileItemsDest.at(pGetSourceTextData->iItem).wsize.c_str();
		bHandled = TRUE;
	}
	return S_OK;
}
    
HRESULT CDestFileList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	//DebugMsg("OnGetItemCountAll - %d items",files->nItems);
	pGetItemCountData->cItems =m_FileItemsDest.size();
	bHandled = TRUE;
	return S_OK;
}

HRESULT CDestFileList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	//DebugMsg("OnGetSourceDataImage - %d",pGetSourceImageData->iData);
	// icon
	if( ( 2 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {
		if (m_FileItemsDest.at(pGetSourceImageData->iItem).isDir)
		{
			if (m_FileItemsDest.at(pGetSourceImageData->iItem).isSelected){
				pGetSourceImageData->szPath = ListIconPaths.szSelectedFolderIcon.c_str();
			} else {
				pGetSourceImageData->szPath = ListIconPaths.szStandardFolderIcon.c_str();
			}
		} else {
			if (m_FileItemsDest.at(pGetSourceImageData->iItem).isSelected){
				pGetSourceImageData->szPath = ListIconPaths.szSelectedFileIcon.c_str();
			} else {
				pGetSourceImageData->szPath = ListIconPaths.szStandardFileIcon.c_str();
			}
		}
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CDestFileList::OnListRefresh(BOOL& bHandled )
{
	errno = 0;
	vector<string> folders = m_FileBrowserDest.GetFolderList();
	if (folders.size() == 1 && errno != 0) {
		DebugMsg("DestFileList", "Permission was denied, or some other error");
		return S_FALSE;;
	}

	DeleteItems(0,m_FileItemsDest.size());	
	m_FileItemsDest.clear();

	vector<string> files = m_FileBrowserDest.GetFileList();
	for(unsigned int x=0;x<folders.size();x++)
	{
		FileInformation fitm;
		fitm.isDir = true;
		fitm.isSelected = false;
		fitm.name = folders.at(x);
		fitm.path = m_FileBrowserDest.GetCurrentPath();
		fitm.wname = strtowstr(fitm.name);
		fitm.wpath = strtowstr(fitm.path);
		m_FileItemsDest.push_back(fitm);
	}
	for(unsigned int x=0;x<files.size();x++)
	{
		FileInformation fitm;
		fitm.isDir = false;
		fitm.isSelected = false;
		fitm.name = files.at(x);
		fitm.path = m_FileBrowserDest.GetCurrentPath();
		fitm.wname = strtowstr(fitm.name);
		fitm.wpath = strtowstr(fitm.path);
		fitm.size = "";
		fitm.wsize = strtowstr(fitm.size);

		struct _stat64 statFileStatus;

		if (fitm.path.substr(0,4).compare("smb:") == 0) {
			string fullPath = m_FileBrowserDest.GetCurrentPath() + "/" + fitm.name;
			int s = smbc_stat(fullPath.c_str(), &statFileStatus); // this could fail, no?	
			if (s < 0) {
				DebugMsg("FileBrowser", "Stat failed [%s] (%d:%s)\n", fullPath.c_str(), errno, strerror(errno));
				m_FileItemsDest.push_back(fitm);
				continue;
			}
		}
		else {
			string fullPath = m_FileBrowserDest.GetCurrentPath() + "\\" + fitm.name;
			_stat64(fullPath.c_str(),&statFileStatus);
		}

		LARGE_INTEGER size;
		size.QuadPart = statFileStatus.st_size;

		if(size.QuadPart <= 1024)
        {
            fitm.size = sprintfaA("%i byte",size.QuadPart);
        }
        else if (size.QuadPart > 1024 && size.QuadPart <= (1024 * 1024))
        {
            fitm.size = sprintfaA("%0.1f kB",(float)size.QuadPart/1024.0f);
        }
        else if (size.QuadPart > (1024 * 1024) && size.QuadPart <= (1024 * 1024 * 1024))
        {
            fitm.size = sprintfaA("%0.1f MB",(float)size.QuadPart/(1024.0f*1024.0f));
        }
        else
        {
            fitm.size = sprintfaA("%0.1f GB",(float)size.QuadPart/(1024.0f*1024.0f*1024.0f));
        }

		fitm.wsize = strtowstr(fitm.size);

		m_FileItemsDest.push_back(fitm);
	}
	sort(m_FileItemsDest.begin(),m_FileItemsDest.end(),DestSortFiles);

	InsertItems(0,m_FileItemsDest.size());

	bHandled = true;
	return S_OK;
}


HRESULT CDestFileList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{

	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg, XM_FILES_FILECHANGE);
	CXuiElement parent;
	CXuiElement parent2;
	this->GetParent(&parent);
	parent.GetParent(&parent2);
	XuiSendMessage( parent2.m_hObj , &xuiMsg );

	bHandled = TRUE;

    return S_OK;
}

HRESULT CDestFileList::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	// Code to fake  button press animation for file manager
	CXuiElement parent;
	CXuiElement parent2;
	CXuiControl m_Button;
	HRESULT TEST = this->GetParent(&parent);
	TEST =parent.GetParent(&parent2);
	if (TEST == S_OK)
	{
		if (parent == NULL)
		{
			DebugMsg("FileList", "No Parent");
		}
	}
	
	HRESULT hr = parent.GetChildById(L"Select", &m_Button);
	if(hr==S_OK)
	{
		XUIMessage xuiMsg;
		XUIMessagePress xuiMsgPress;
		XuiMessagePress( &xuiMsg, &xuiMsgPress, XUSER_INDEX_ANY );
		// Send the XM_PRESS message.
		TEST = XuiSendMessage( m_Button.m_hObj, &xuiMsg );
		if (TEST == S_OK)
		{
			DebugMsg("FileList", "Sent Message");
		}
	}

	FileInformation item = m_FileItemsDest.at(GetCurSel());
	string ext = make_lowercaseA(FileExtA(item.name));
	
	if (item.isDir)
	{
		if (bIgnoreStoredCredentials) {
			string curPath = m_FileBrowserDest.GetCurrentPath();
			if (curPath.substr(0,4).compare("smb:") == 0) {
				m_FileBrowserDest.CD(item.name);
				string user, password, share;
				string newPath = m_FileBrowserDest.GetRawSmbPath(user, password, share);
				if (user.compare("") != 0) {
					GetCredentials(newPath, user, password, share);
				}
				m_FileBrowserDest.UpDirectory();
			}
			bIgnoreStoredCredentials = false;
			bHandled = TRUE;
			return S_OK;
		}

		if (!Shift) {
			m_FileBrowserDest.CD(item.name);
			
			BOOL Temp = FALSE;
		
			SetCurSel(0);
			SetTopItem(0);
			if (OnListRefresh(Temp) == S_FALSE) {
				m_FileBrowserDest.UpDirectory();
			}
		}

		XUIMessage xuiMsg;
		XuiMessage(&xuiMsg, XM_FILES_DIRCHANGE);
		XuiSendMessage( parent2.m_hObj , &xuiMsg );
	
	} else {
		XUIMessage xuiMsg;
		XuiMessage(&xuiMsg, XM_FILES_FILEPRESS);
		XuiSendMessage( parent2.m_hObj , &xuiMsg );
	}

	bHandled = TRUE;
	return S_OK;
}

HRESULT CDestFileList::OnDoBack( BOOL& bHandled )
{
	m_FileBrowserDest.UpDirectory();

	BOOL Temp = FALSE;

	SetCurSel(0);
	SetTopItem(0);
	OnListRefresh(Temp);

	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg, XM_FILES_DIRCHANGE);
	CXuiElement parent;
	CXuiElement parent2;
	this->GetParent(&parent);
	parent.GetParent(&parent2);
	XuiSendMessage( parent2.m_hObj , &xuiMsg );

	return S_OK;
}

