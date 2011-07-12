#pragma once
#include "../../Tools/Debug/Debug.h"
#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../Abstracts/HideableScene/HideableScene.h"
#include "../../Tools/Threads/ThreadLock.h"
#include "../../Tools/TextScroller/TextScroller.h"
class CScnRssReader:public HideableScene,public iStatusDisplayer
{
protected :
	ThreadLock m_Lock;
	string m_CurrentFeedTitle;
	string m_CurrentItemTitle;
	string m_CurrentItemDescription;
	string m_CurrentItemDate;
	string m_CurrentItemComplete;

	string m_FontWhite;
	string m_FontBlack;

	string feedTitleColor, itemTitleColor, itemDescColor, itemDateColor, fontSize, titleFontSize;
	
	bool showFeedTitle, showItemTitle, showItemContent, showItemDate, showItemComplete;
	CXuiHtmlElement m_Elem_FeedTitle;
	CXuiHtmlElement m_Elem_ItemTitle;
	CXuiHtmlElement m_Elem_ItemContent;
	CXuiHtmlElement m_Elem_ItemDate;
	CXuiHtmlElement m_Elem_ItemComplete;
	
	
	void RefreshGui();
	void PlayAnim();
	void CreateFullString();
	void FormatDescriptionString();
public :
	
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
	XUI_END_MSG_MAP()
	
	void setStatus(StatusInfo inf);

	XUI_IMPLEMENT_CLASS(CScnRssReader,L"ScnRssReader",XUI_CLASS_SCENE);

    HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
   
};
