#pragma once
#include "../../Generic/xboxtools.h"

// DebugMessage Control
#define SHOW_DEBUG	0      // 0 = DebugOff   1 = DebugOn

class RssManager
{
public:
	static RssManager& getInstance()
	{
		static RssManager singleton;
		return singleton;
	}

	string feedTitleColor, itemTitleColor, itemBodyColor, itemDateColor, feedTitleFontSize, itemFontSize;

	void LoadSettingsFromXML();

	// Public Accessor Functions - get
	string GetFeedTitleColor()
	{
		return feedTitleColor;
	}
	string GetItemTitleColor()
	{
		return itemTitleColor;
	}
	string GetItemBodyColor()
	{
		return itemBodyColor;
	}
	string GetItemDateColor()
	{
		return itemDateColor;
	}
	string GetFeedTitleFontSize()
	{
		return feedTitleFontSize;
	}
	string GetItemFontSize()
	{
		return itemFontSize;
	}

private:
	int nDebugFlag;

	RssManager();			                       // Private constructor
	~RssManager() {}
	RssManager(const RssManager&);                 // Prevent copy-construction
	RssManager& operator=(const RssManager&);      // Prevent assignment
};
