#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/SQLite/FSDSql.h"
#include "../../Abstracts/ConfigurableList/ConfigurableList.h"
#include "../../../stdafx.h"


class CTUPathList : ConfigurableList
{
private :

public:

	static vector<TitleUpdatePath*> TUPaths;
	wstring temp;

	XUI_IMPLEMENT_CLASS( CTUPathList, L"TUPathList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
	XUI_END_MSG_MAP()


	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnListRefresh( BOOL& bHandled );
};
