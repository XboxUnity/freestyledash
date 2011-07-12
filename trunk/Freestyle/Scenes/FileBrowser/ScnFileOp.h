#pragma once
#include "../../Tools/Generic/tools.h"

#include "../../Tools/Threads/copythread.h"

extern string CD_Source;
extern string CD_Dest;
extern CD_ActionNames CD_Action;
extern bool CD_Done;

class CScnFileOp :
	public CXuiSceneImpl
{
private :
	XOVERLAPPED overlapped;
	LPCWSTR* btnOk;
	MESSAGEBOX_RESULT mbresult;

public:
	CXuiTextElement m_from;
	CXuiTextElement m_to;
	CXuiTextElement m_stats;
	CXuiTextElement m_title;
	CXuiTextElement m_fileCount;
	CXuiTextElement m_sizeComplete;
	CXuiTextElement m_percentComplete;
	CXuiTextElement m_transSpeed;
	CXuiTextElement m_curFile;
	CXuiTextElement m_fileSizeComplete;
	CXuiTextElement m_filePercentCom;
	CXuiTextElement m_ETA;
	CXuiProgressBar m_sub1pb;
	CXuiProgressBar m_sub2pb;
	CXuiControl m_cancel;

	LARGE_INTEGER BytesLastTick;
	LARGE_INTEGER BytesPerSec;

	bool Completed;

	XUI_IMPLEMENT_CLASS( CScnFileOp, L"ScnFileOp", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	
	void DisplayResult(string & result);
};
