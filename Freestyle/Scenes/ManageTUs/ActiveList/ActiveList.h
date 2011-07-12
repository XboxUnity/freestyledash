#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/SQLite/FSDSql.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"
#include "../../../stdafx.h"

#define XM_REFRESH  XM_USER+1

#define XUI_ON_XM_REFRESH(MemberFunc)\
    if (pMessage->dwMessage == XM_REFRESH)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }

class CActiveList : ConfigurableList
{
private :
	BOOL loaded;
	struct {
			wstring szActiveTUIcon;
			wstring szDisabledTUIcon;
	} ListIconPaths;	

public:

	static vector<SQLTitleUpdate*> ActiveTU;
	void preparelist();
	DWORD GetCurrentTitleID();
	void Refresh();
	wstring temp;

	XUI_IMPLEMENT_CLASS( CActiveList, L"ActiveList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_REFRESH(OnListRefresh)
	XUI_END_MSG_MAP()

	
	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnListRefresh( BOOL& bHandled );
};
