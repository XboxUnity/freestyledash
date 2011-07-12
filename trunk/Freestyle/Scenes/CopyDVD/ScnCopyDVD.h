#pragma once

#include "../../Tools/Generic/tools.h"
#include "../../Tools/Monitors/DVD/DVDMonitor.h"
#include "../../Tools/DVDInfo/DVDItem.h"

using namespace std;

class CScnCopyDVD : public CXuiSceneImpl, public iDVDObserver, public iDVDItemRequester
{
private : 
	XOVERLAPPED keyboard;
	WCHAR * buffer;

protected:
    
	CXuiScene m_CopyDVD;

	bool isNested;

    CXuiControl m_back;
	CXuiTextElement m_TotalFileProgress, m_TotalFileSizeProgress, m_FileSizeComplete;
	CXuiTextElement m_FileName;
	CXuiTextElement m_TitleName;
	CXuiTextElement m_DestinationPath;
	CXuiTextElement m_OverallPercent;
	CXuiTextElement m_OverallTitle;

	CXuiControl m_OK;
	CXuiControl m_Cancel;
	CXuiControl m_border1;
	CXuiControl m_border2;
	CXuiControl m_CopyName;
	CXuiControl m_CopyPath;
	CXuiCheckbox m_IncludeUpdate;
	CXuiTextElement m_PathText;
	CXuiTextElement m_NameText;
	DVDItem* Item;

    CXuiProgressBar m_TotalProgressBar, m_FileProgressBar;

	smc smc;

	LPCWSTR* btnOk;
	
    XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
        XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
    XUI_END_MSG_MAP()

    HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
    HRESULT OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled);
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);

	void ExtractCompleted(DVDItem* itm);
	void UpdateFileCount(DVDItem*);
	void UpdateFile();
	void UpdateOverall();
	void getDVDTitleXeX();
	void SetDVD();
	void handle360GameDVDInsertedEvent();
	void handleXbox1GameDVDInsertedEvent();
	void handleTrayOpenEvent();
	void handleTrayCloseEvent();
	void SetProgressShow(bool show);
	void ReadDVD(CONTENT_DVD_TYPE type, string pathS);

public:
    XUI_IMPLEMENT_CLASS( CScnCopyDVD, L"ScnCopyDVD", XUI_CLASS_TABSCENE )		
	string copypath;
	string copydvdname;
	wstring dvdName;
    CScnCopyDVD();
    ~CScnCopyDVD();
};
