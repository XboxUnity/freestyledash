#pragma once

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Generic/tools.h"
#include "XML/TUXml.h"
#include "ActiveList/ActiveList.h"
#include "../../Tools/Managers/TitleUpdates/TUDownloadManager.h"

using namespace std;


class CScnManageTUs : CXuiSceneImpl, public TUDlMReq

{
private :

public:
	
	CXuiScene m_TitleManager;
	CXuiList m_ActiveList, m_TUPaths;
	CXuiControl m_Download, m_DeleteUpdate,m_Upload, m_Back;
	CXuiTextElement m_PathLabel, m_ActiveLabel;
	CXuiElement m_ActiveBackground;
	
	bool isNested;

	MESSAGEBOX_RESULT mbresult;
	XOVERLAPPED overlapped;
	LPCWSTR* btnOk;


	XUI_IMPLEMENT_CLASS( CScnManageTUs, L"ScnManageTUs", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
		XUI_ON_XM_MSG_RETURN( OnMsgReturn )
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnMsgReturn(XUIMessageMessageBoxReturn *pXUIMessageMessageBoxReturn, BOOL &bHandled);

	void showPaths(bool show);
	void done();

	//---------- for download requestor stuff ------ //

	
	TitleUpdatePath* TUPath;
	
};
