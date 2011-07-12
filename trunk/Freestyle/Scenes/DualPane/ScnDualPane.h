#pragma once
#include <xmedia2.h>
#include "../../Tools/Generic/tools.h"

#include "SourceFileList/SourceFilelist.h"
#include "DestFileList/DestFileList.h"
#include "../../Tools/Managers/FileOperation/FileCallBackItem.h"

using namespace std;

class CScnDualPane :
	public CXuiSceneImpl, public iFileCallBackItemRequester
{
private :
	WCHAR result[1024];
	LPCWSTR* btnYesNo;
	LPCWSTR* btnOk;
	XOVERLAPPED overlapped;
	int keyboardMode;
	FileCallBackItem* Item;
	wstring szTempString;

protected:
	CXuiScene m_ChildScn;

	//File Browser
	CXuiTextElement m_copytext, m_Prep;
	CXuiList m_SrcFileList;
	CXuiList m_DestFileList;
	CXuiControl m_copy;
	CXuiControl m_Move;
	CXuiControl m_rename;
	CXuiControl m_delete;
	CXuiControl m_makedir;
	CXuiControl m_RB, m_LB;
    CXuiControl m_Mark;
	CXuiElement m_background_Controls, m_background_SelectedFiles, m_background_FilesSrc, m_background_FilesDest;
	CXuiControl m_Launch;
	CXuiImageElement m_ArrowRight, m_ArrowRight1, m_ArrowLeft, m_ArrowLeft1;
	CXuiTextElement m_Right, m_Left;
	bool RBShift;
	bool LBShift;
	bool Shift;
	bool copying;
	bool leftRename;
	bool leftMakeDir;
	
	//Global
	CXuiTextElement m_title;
	CXuiControl m_choosedrive;
	CXuiVideo m_video;
	CXuiControl m_back;

	//Progress Bar
	CXuiTextElement m_TotalFileProgress, m_TotalFileSizeProgress, m_FileSizeComplete;
	CXuiTextElement m_FileName;
	CXuiTextElement m_OverallPercent;
	CXuiTextElement m_OverallTitle;
	CXuiTextElement m_LabelAction;
	CXuiProgressBar m_TotalProgressBar, m_FileProgressBar;
	CXuiControl m_Cancel;

	string copysource, copysourcename, copydestination;
	string copyfile;
	bool IsCut, isNested;
	int selDestItemCount, selSrcItemCount, mb_Type, deleteSelection;

	int OverlapFunc;
	IXMedia2XmvPlayer* xmvPlayer;
	WCHAR DefaultName[1024];
	WCHAR MessageText[1024];


	string GetSelectedFile(int iItem);
	string GetSelectedFileName(int iItem);

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_FILES_FILECHANGE( OnFilesChange )
		XUI_ON_XM_FILES_DIRCHANGE( OnDirChange )
		XUI_ON_XM_FILES_FILEPRESS( OnFilePress )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_KEYUP( OnKeyUp )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
		XUI_ON_XM_SMB_PERMISSION_DENIED( OnSmbPermissionDenied )
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnFilesChange( BOOL& bHandled );
	HRESULT OnFilePress( BOOL& bHandled );
	HRESULT OnDirChange( BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);
	HRESULT OnSmbPermissionDenied( Credentials* cred, BOOL& bHandled );

	void UpdateOverall(FileCallBackItem* Itm);
	void UpdateFile(FileCallBackItem* Itm);
	void SetProgressShow(bool show);
	void SetPrepShow(bool show);
	void SetShift(bool show);
	void SetArrow(int arr);
	void Complete();
	void Copy();
	void ResetShift();

public:
	void UpdateInfo(FileCallBackItem*);
	XUI_IMPLEMENT_CLASS( CScnDualPane, L"ScnDualPane", XUI_CLASS_TABSCENE )
	CScnDualPane(){ Shift = false; };
	~CScnDualPane();
};
