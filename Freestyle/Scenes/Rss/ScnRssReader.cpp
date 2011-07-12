#include "stdafx.h"

#include "../../Tools/Debug/Debug.h"
#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../Abstracts/HideableScene/HideableScene.h"
#include "../../Tools/Threads/ThreadLock.h"
#include "ScnRssReader.h"
#include "../../Tools/TextScroller/TextScroller.h"
#include "../../Tools/Managers/Rss/RssManager.h"

HRESULT CScnRssReader::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	DebugMsg("ScnRssReader","Init");
	//Get childs
	HRESULT hr = GetChildById(L"FeedTitle",&m_Elem_FeedTitle);
	showFeedTitle = hr == S_OK;
	hr = GetChildById(L"ItemTitle",&m_Elem_ItemTitle);
	showItemTitle = hr == S_OK;
	hr = GetChildById(L"ItemDescription",&m_Elem_ItemContent);
	showItemContent = hr == S_OK;
	hr = GetChildById(L"ItemDate",&m_Elem_ItemDate);
	showItemDate = hr == S_OK;
	hr = GetChildById(L"ItemComplete",&m_Elem_ItemComplete);
	showItemComplete = hr == S_OK;

	//Reset Default Value
	m_CurrentFeedTitle ="";
	m_CurrentItemTitle= "";
	m_CurrentItemDescription = "";
	m_CurrentItemDate = "";
	m_CurrentItemComplete = "";

	//Set Default Colors
	feedTitleColor = RssManager::getInstance().GetFeedTitleColor();
	itemTitleColor = RssManager::getInstance().GetItemTitleColor();
	itemDescColor = RssManager::getInstance().GetItemBodyColor();
	itemDateColor = RssManager::getInstance().GetItemDateColor();

	titleFontSize = RssManager::getInstance().GetFeedTitleFontSize();
	fontSize = RssManager::getInstance().GetItemFontSize();

	RefreshGui();
	TextScroller::getInstance().RegisterDisplayer(this);

	return S_OK;
}


void CScnRssReader::setStatus(StatusInfo inf)
{
	m_CurrentFeedTitle = "<font color=\"" + feedTitleColor + "\" size=\"" + titleFontSize + "\" name=\"ConsoleFont\">" + "hello" /*inf.feedTitle*/ + "</font>";
	m_CurrentItemTitle= "<font color=\"" + itemTitleColor + "\" size=\"" + fontSize + "\" name=\"ConsoleFont\">" + "hi" /*inf.itemTitle*/ + "</font>";
	m_CurrentItemDescription = "<font color=\"" + itemDescColor + "\" size=\"" + fontSize + "\" name=\"ConsoleFont\">" + "how" /*inf.itemDescription*/ + "</font>";
	m_CurrentItemDate = "<font color=\"" + itemDateColor + "\" size=\"" + fontSize + "\" name=\"ConsoleFont\">" + "are you" /*inf.itemPubDate*/ + "</font>";

	FormatDescriptionString();

	CreateFullString();
}

void CScnRssReader::RefreshGui()
{
	m_Lock.Lock();
	try
	{
		if(showFeedTitle)
		{
			m_Elem_FeedTitle.SetText(strtowstr(m_CurrentFeedTitle).c_str());
		}
		if(showItemTitle)
		{
			m_Elem_ItemTitle.SetText(strtowstr(m_CurrentItemTitle).c_str());
		}
		if(showItemContent)
		{
			m_Elem_ItemContent.SetText(strtowstr(m_CurrentItemDescription).c_str());
		}
		if(showItemDate)
		{
			m_Elem_ItemDate.SetText(strtowstr(m_CurrentItemDate).c_str());
		}
		if(showItemComplete)
		{
			wstring szItem = strtowstr(m_CurrentItemComplete);
			m_Elem_ItemComplete.SetText(szItem.c_str());
		}
	}
	catch(...)
	{
		DebugMsg("ScnRssReader","Error outputting to gui");
	}
	m_Lock.Unlock();

	PlayAnim();
}

void CScnRssReader::CreateFullString()
{
	if( m_CurrentItemDescription.size() > 200 )
	{
		m_CurrentItemDescription = m_CurrentItemDescription.substr(0,200);

		int srcpos = m_CurrentItemDescription.find("<i>");

		if (srcpos > -1) 
		{
			int srcpos2 = m_CurrentItemDescription.find("</i>"); 
			
			if (srcpos2 > -1)
				m_CurrentItemDescription.append("...");
			else
				m_CurrentItemDescription.append("...</i>");
		}
		else
			m_CurrentItemDescription.append("...");
	}

	m_CurrentItemComplete = ""; // Reset String

	m_CurrentItemComplete.append(m_CurrentItemTitle);

	m_CurrentItemComplete.append("&nbsp;&nbsp;&nbsp;&nbsp;");

	m_CurrentItemComplete.append(m_CurrentItemDescription);

	m_CurrentItemComplete.append("&nbsp;&nbsp;&nbsp;&nbsp;");

	m_CurrentItemComplete.append(m_CurrentItemDate);

	RefreshGui();
}

void CScnRssReader::FormatDescriptionString()
{
	//Properly Format Quotes
	m_CurrentItemDescription = str_replaceallA(m_CurrentItemDescription,"[QUOTE]","<i>\"");
	m_CurrentItemDescription = str_replaceallA(m_CurrentItemDescription,"[/QUOTE]","\"</i>");
}

void CScnRssReader::PlayAnim()
{
	int nFrameStart, nFrameEnd;
	FindNamedFrame(L"Start", &nFrameStart);
	FindNamedFrame(L"LoopFeed", &nFrameEnd);
	PlayTimeline(nFrameStart, nFrameStart, nFrameEnd, FALSE, FALSE);
}