#pragma once
#include "RssFeed.h"

class RssReader 
{
private :
	RssReader(){};          // Private constructor
  ~RssReader() {}
  RssReader(const RssReader&);                 // Prevent copy-construction
  RssReader& operator=(const RssReader&);      // Prevent assignment
  vector<RssFeed*> m_Feeds;
public :
	static RssReader& getInstance()
	{
		static RssReader singleton;
		return singleton;
	}
	void AddRssFeed(string url)
	{
		int index = m_Feeds.size();
		RssFeed* fd= new RssFeed(url,index);
		m_Feeds.push_back(fd);
	}

};