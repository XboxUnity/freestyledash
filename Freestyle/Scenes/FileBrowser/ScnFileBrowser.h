#pragma once
#include <xmedia2.h>
#include "../../Tools/Generic/tools.h"

#include "FilesList/fileslist.h"

using namespace std;

extern bool FileBrowserSelectFile;
extern bool FileBrowserSelected;
extern bool FileBrowserSelectFolder;
extern string FileBrowserFilename;
extern IXMedia2XmvPlayer* xmvPlayer;

class CScnFileBrowser :
	public CXuiSceneImpl
{
private :
	WCHAR result[1024];
	XOVERLAPPED overlapped;
	LPCWSTR* btnYesNo;
	LPCWSTR* btnOk;
public:
	CXuiScene m_ChildScn;
	
	//File Browser
	CXuiTextElement m_copytext;
	CXuiList m_filelist;
	CXuiControl m_copy;
	CXuiControl m_cut;
	CXuiControl m_paste;
	CXuiControl m_rename;
	CXuiControl m_delete;
	CXuiControl m_makedir;
    CXuiControl m_dir;
	CXuiElement m_background_Controls, m_background_SelectedFiles, m_background_Files;
	
	//Global
	CXuiTextElement m_title;
	CXuiControl m_choosedrive;
	CXuiVideo m_video;
	CXuiControl m_Back;
	CXuiControl m_exitscene;

	//Progress Bar
	CXuiTextElement m_TotalFileProgress, m_TotalFileSizeProgress, m_FileSizeComplete;
	CXuiTextElement m_FileName;
	CXuiTextElement m_OverallPercent;
	CXuiTextElement m_OverallTitle;
	CXuiProgressBar m_TotalProgressBar, m_FileProgressBar;
	CXuiControl m_Cancel;

	string copysource, copysourcename, copydestination;
	string copyfile;
	bool IsCut, isNested;

	int OverlapFunc, deleteSelection;


	string GetSelectedFile();
	string GetSelectedFileName();
	XUI_IMPLEMENT_CLASS( CScnFileBrowser, L"ScnFileBrowser", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_FILES_FILECHANGE( OnFilesChange )
		XUI_ON_XM_FILES_DIRCHANGE( OnDirChange )
		XUI_ON_XM_FILES_FILEPRESS( OnFilePress )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnFilesChange( BOOL& bHandled );
	HRESULT OnFilePress( BOOL& bHandled );
	HRESULT OnDirChange( BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);
	void CScnFileBrowser::UpdateOverall();
	void CScnFileBrowser::UpdateFile();
	void CScnFileBrowser::SetProgressShow(bool show);
};
