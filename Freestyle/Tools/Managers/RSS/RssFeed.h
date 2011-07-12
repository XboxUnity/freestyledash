#pragma once

#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "../../TextScroller/TextScroller.h"

class RssFeed: public iHttpItemRequester,public iStatusInformer
{
private :
	string m_Url;
	DWORD m_LastTickUpdated ;
	string m_Title;
	vector<string> m_itemsTitle;
	vector<string> m_itemsContent;
	vector<string> m_itemsPubDate;
	int m_CurrentDisplayIndex;
	int m_FeedIndex;
	bool m_WaitingForDownload;
	string getRssFeedTempPath(int nIndex);
public :
	 RssFeed(string url,int feedIndex)
	 {
		 m_FeedIndex = feedIndex;
		 m_Url = url;
		m_Title = "";
		m_CurrentDisplayIndex = 0;
		 m_LastTickUpdated = 0;
		 m_WaitingForDownload= false;
		 TextScroller::getInstance().RegisterInformer(this);
		 RequestDownload();
	 }
	 void RequestDownload();
	 void DownloadCompleted(HttpItem* itm);
	 StatusInfo getStatus();
};