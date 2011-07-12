#include "stdafx.h"

#include "RssFeed.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/HTTPDownloader.h"
#include "XML/RssXML.h"
#include "../../Settings/Settings.h"
string RssFeed::getRssFeedTempPath(int nIndex)
{
	string dataPath = "game:\\Rss";

	if(!FileExistsA(dataPath))
	{
		//  DebugMsg("Creating %s",dataPath.c_str());
		_mkdir(dataPath.c_str());
	}
	dataPath = sprintfaA("game:\\Rss\\%s-%d.dat", "rssfeed", nIndex);
	dataPath = str_replaceallA(dataPath,"\\\\","\\");

	if(FileExistsA(dataPath))
	{
		unlink(dataPath.c_str());
	}
	return dataPath;

}

void RssFeed::RequestDownload()
{
	m_WaitingForDownload = true;
	string rssPath = getRssFeedTempPath(m_FeedIndex);
	HttpItem* itm = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)m_Url.c_str(), (CHAR*)rssPath.c_str());
	itm->setTag("RSSFeed");
	itm->SetPriority(HTTP_PRIORITY_HIGH);
	itm->setRequester(this);

	HTTPDownloader::getInstance().AddToQueue(itm);
}
StatusInfo RssFeed::getStatus() 
{
	StatusInfo retVal;
	retVal.feedTitle = m_Title;
	retVal.itemTitle = "";
	retVal.itemDescription = "";
	retVal.itemPubDate = "";
	try
	{
	if(m_itemsTitle.size() == 0)
	{
		if(m_WaitingForDownload)
		{
			retVal.itemDescription = "Downloading Feed...";
			
		}
		else
		{
			retVal.itemDescription = "No items...";
			
		}
	}
	else
	{
		int nSize = m_itemsTitle.size();
		if(m_FeedIndex >= nSize)
		{
			m_FeedIndex= 0;
		}
		retVal.itemTitle = m_itemsTitle.at(m_FeedIndex);
		retVal.itemDescription = m_itemsContent.at(m_FeedIndex);
		retVal.itemPubDate = m_itemsPubDate.at(m_FeedIndex);
		m_FeedIndex ++;
	}
	}
	catch(...)
	{
		DebugMsg("RssFeed","Error generating status");
	}
	return retVal;
}
void RssFeed::DownloadCompleted(HttpItem* itm)
{
	m_LastTickUpdated =GetTickCount();
	m_WaitingForDownload = false;
	if(itm->getResponseCode() == 200)
	{
		if(FileExists(itm->getSaveAsPath()))
		{
			//Parse XML
			RssXML xml;
			ATG::XMLParser parser;
			parser.RegisterSAXCallbackInterface(&xml);
			parser.ParseXMLFile(itm->getSaveAsPath().c_str());

			if(xml.gotTitle)
			{
				m_Title = xml.m_Title;
			}
			int nSize = xml.m_rssItems.size();
			for(int x=0;x<nSize;x++)
			{
				m_itemsPubDate.push_back(xml.m_rssItems.at(x).szRssDate);
				m_itemsContent.push_back(xml.m_rssItems.at(x).szRssDescription);
				m_itemsTitle.push_back(xml.m_rssItems.at(x).szRssTitle);
			}
		}
	}
}