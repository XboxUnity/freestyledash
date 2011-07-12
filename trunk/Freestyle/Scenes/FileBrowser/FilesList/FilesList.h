#pragma once
#include "../../../Tools/Generic/tools.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"
#include "../../../Tools/FileBrowser/FileBrowser.h"

#define XM_FILES_REFRESH  XM_USER+1

#define XUI_ON_XM_FILES_REFRESH(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_REFRESH)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }


#define XM_FILES_DIRCHANGE  XM_USER+2

#define XUI_ON_XM_FILES_DIRCHANGE(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_DIRCHANGE)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }


#define XM_FILES_FILECHANGE  XM_USER+3

#define XUI_ON_XM_FILES_FILECHANGE(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_FILECHANGE)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }

#define XM_FILES_FILEPRESS  XM_USER+4

#define XUI_ON_XM_FILES_FILEPRESS(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_FILEPRESS)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }

#define XM_FILES_DOBACK  XM_USER+5

#define XUI_ON_XM_FILES_DOBACK(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_DOBACK)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }


class CFilesList : ConfigurableList
{
private :
	struct {
		wstring szStandardFileIcon;
		wstring szStandardFolderIcon;
	} ListIconPaths;
public:
	static vector<FileItem> m_FileItems;
	static FileBrowser m_FileBrowser;

    XUI_IMPLEMENT_CLASS( CFilesList, L"FilesList", XUI_CLASS_LIST );

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
	XUI_END_MSG_MAP()

	//LIST_DATA m_ListData;
	    
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
