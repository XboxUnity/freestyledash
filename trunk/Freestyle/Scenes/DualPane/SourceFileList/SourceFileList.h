#pragma once
#include "../../../Tools/Generic/tools.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"

#include "../../../Tools/FileBrowser/FileBrowser.h"
#include "../../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../../Tools/Generic/CustomMessages.h"

class CSrcFileList : ConfigurableList
{
private :
	struct {
		wstring szStandardFileIcon;
		wstring szStandardFolderIcon;
		wstring szSelectedFileIcon;
		wstring szSelectedFolderIcon;
	} ListIconPaths;

	static bool bIgnoreStoredCredentials;
public:
	static vector<FileInformation> m_FileItemsSrc;
	static FileBrowser m_FileBrowserSrc;
	static bool Shift;

    XUI_IMPLEMENT_CLASS( CSrcFileList, L"SrcFilesList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
        XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_FILES_REFRESH( OnListRefresh )
		XUI_ON_XM_TIMER( OnTimer )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
        XUI_ON_XM_FILES_DOBACK( OnDoBack )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_KEYUP( OnKeyUp )
	XUI_END_MSG_MAP()

	//LIST_DATA m_ListData;
	HRESULT OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled);	    
	HRESULT OnKeyUp(XUIMessageInput *pInputData, BOOL& bHandled);
	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnListRefresh( BOOL& bHandled );
	HRESULT OnDoBack( BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
};
