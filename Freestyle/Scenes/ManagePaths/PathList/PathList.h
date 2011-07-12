#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"
#include "../../../Tools/Settings/Settings.h"
#include "../../../Tools/SQLite/FSDSql.h"

#define XM_REFRESH  XM_USER+1

#define XUI_ON_XM_REFRESH(MemberFunc)\
    if (pMessage->dwMessage == XM_REFRESH)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }


class CPathList : ConfigurableList
{
private :
	struct {
			wstring szStandardFolderIcon;
	} ListIconPaths;

	int m_currentListSize;

	wstring temp;
public:
	static vector<ScanPath> ScanPaths;
	vector<SQLTab*> tabID;

	XUI_IMPLEMENT_CLASS( CPathList, L"PathList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_REFRESH(OnListRefresh)
	XUI_END_MSG_MAP()

	CRITICAL_SECTION lock;

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnListRefresh( BOOL& bHandled );
};
