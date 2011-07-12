#pragma once

#include "../../../../Tools/Generic/tools.h"

class CVideoList : CXuiListImpl
{
private :

public:

    XUI_IMPLEMENT_CLASS( CVideoList, L"VideoList", XUI_CLASS_LIST );

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage);
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
	XUI_END_MSG_MAP()


	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnListRefresh( BOOL& bHandled );
};
