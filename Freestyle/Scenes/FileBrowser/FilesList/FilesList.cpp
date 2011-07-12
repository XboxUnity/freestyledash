#include "stdafx.h"

#include "FilesList.h"

#include <algorithm>
#include <sys/stat.h>
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/FileBrowser/FileBrowser.h"
using namespace std;

vector<FileItem> CFilesList::m_FileItems;
FileBrowser CFilesList::m_FileBrowser;
bool SortFiles(const FileItem  left, const FileItem  right) 
{ 
	if(left.IsDir && left.name == "..")
	{

		return true;
	}
	if(right.IsDir && right.name == "..")
	{
		return false;
	}
	if (left.IsDir != right.IsDir) return left.IsDir;

	int res = _stricmp(left.name.c_str(),right.name.c_str());
	if (res < 0) return true;
	return false;
}

HRESULT CFilesList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	LoadSettings("FilesList", *this);

	ListIconPaths.szStandardFileIcon = strtowstr(GetSetting("STANDARDFILEPATH", ""));
	ListIconPaths.szStandardFolderIcon = strtowstr(GetSetting("STANDARDFOLDERPATH", ""));
	
	DebugMsg("FileList","CFilesList::OnInit");
	return S_OK;
}

HRESULT CFilesList::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{

	return S_OK;
}
    

// Gets called every frame
HRESULT CFilesList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	//DebugMsg("OnGetSourceDataText - %d",pGetSourceTextData->iData);
	// filename
	if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {
	
		pGetSourceTextData->szText = m_FileItems.at(pGetSourceTextData->iItem).wname.c_str();
		bHandled = TRUE;
	}
	// extra text
	if( ( 1 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {
		pGetSourceTextData->szText = m_FileItems.at(pGetSourceTextData->iItem).wsize.c_str();
		bHandled = TRUE;
	}
	return S_OK;
}
    
HRESULT CFilesList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	//DebugMsg("OnGetItemCountAll - %d items",files->nItems);
	pGetItemCountData->cItems =m_FileItems.size();
	bHandled = TRUE;
	return S_OK;
}

HRESULT CFilesList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	//DebugMsg("OnGetSourceDataImage - %d",pGetSourceImageData->iData);
	// icon
	if( ( 2 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {
		if (m_FileItems.at(pGetSourceImageData->iItem).IsDir)
		{
			pGetSourceImageData->szPath = ListIconPaths.szStandardFolderIcon.c_str();
		} else {
			pGetSourceImageData->szPath = ListIconPaths.szStandardFileIcon.c_str();
		}
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CFilesList::OnListRefresh(BOOL& bHandled )
{
	DeleteItems(0,m_FileItems.size());
	
	m_FileItems.clear();
	vector<string> folders = m_FileBrowser.GetFolderList();
	vector<string> files = m_FileBrowser.GetFileList();
	for(unsigned int x=0;x<folders.size();x++)
	{
		FileItem fitm;
		fitm.IsDir = true;
		fitm.name = folders.at(x);
		fitm.CreateWStrs();
		m_FileItems.push_back(fitm);
	}
	for(unsigned int x=0;x<files.size();x++)
	{
		FileItem fitm;
		fitm.IsDir = false;
		fitm.name = files.at(x);
		string fullPath = m_FileBrowser.GetCurrentPath() + "\\" + fitm.name;
		struct stat statFileStatus;
	
		stat(fullPath.c_str(),&statFileStatus);
		
		fitm.size.QuadPart = statFileStatus.st_size;

		fitm.CreateWStrs();
		m_FileItems.push_back(fitm);
	}
	sort(m_FileItems.begin(),m_FileItems.end(),SortFiles);

	InsertItems(0,m_FileItems.size());

	bHandled = true;
	return S_OK;
}


HRESULT CFilesList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{

	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg, XM_FILES_FILECHANGE);
	HXUIOBJ hObjParent;
	this->GetParent(&hObjParent);
	XuiSendMessage( hObjParent , &xuiMsg );

	bHandled = TRUE;

    return S_OK;
}

HRESULT CFilesList::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	// Code to fake  button press animation for file manager
	CXuiElement parent;
	CXuiControl m_Button;
	this->GetParent(&parent);

	HRESULT hr = parent.GetChildById(L"Select", &m_Button);
	if(hr==S_OK)
	{
		XUIMessage xuiMsg;
		XUIMessagePress xuiMsgPress;
		XuiMessagePress( &xuiMsg, &xuiMsgPress, XUSER_INDEX_ANY );
		// Send the XM_PRESS message.
		XuiSendMessage( m_Button.m_hObj, &xuiMsg );
	}

	FileItem  item = m_FileItems.at(GetCurSel());
	
	if (item.IsDir)
	{
		m_FileBrowser.CD(item.name);
		
		BOOL Temp = FALSE;
	
		SetCurSel(0);
		SetTopItem(0);
		OnListRefresh(Temp);

		XUIMessage xuiMsg;
		XuiMessage(&xuiMsg, XM_FILES_DIRCHANGE);
		HXUIOBJ hObjParent;
		this->GetParent(&hObjParent);
		XuiSendMessage( hObjParent , &xuiMsg );

	} else {
		XUIMessage xuiMsg;
		XuiMessage(&xuiMsg, XM_FILES_FILEPRESS);
		HXUIOBJ hObjParent;
		this->GetParent(&hObjParent);
		XuiSendMessage( hObjParent , &xuiMsg );
	}

	bHandled = true;
	return S_OK;
}

HRESULT CFilesList::OnDoBack( BOOL& bHandled )
{
	m_FileBrowser.UpDirectory();

	BOOL Temp = FALSE;

	SetCurSel(0);
	SetTopItem(0);
	OnListRefresh(Temp);

	XUIMessage xuiMsg;
	XuiMessage(&xuiMsg, XM_FILES_DIRCHANGE);
	HXUIOBJ hObjParent;
	this->GetParent(&hObjParent);
	XuiSendMessage( hObjParent , &xuiMsg );

	return S_OK;
}
