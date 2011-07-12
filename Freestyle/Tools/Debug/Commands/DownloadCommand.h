#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../HTTP/HTTPDownloader.h"

class DownloadCommand:public DebugCommand,public iHttpItemRequester
{
public :
	void DownloadCompleted(HttpItem* currentItem)
	{
		DebugMsg("DebugCommand","DEBUG - Done Downloading %s to %s",currentItem->getFullUrl().c_str(),currentItem->getSaveAsPath().c_str());
	}
	DownloadCommand()
	{
		m_CommandName = "Download";
	}
	void Perform(string parameters)
	{
		//string url = parameter
		if(parameters.size() == 0)
		{
			DebugMsg("DebugCommand","--Download URL SAVEAS");
		}
		else
		{
			string url = "";
			string saveas = "";
			int startpos = parameters.find(" ");
			if(startpos != -1)
			{
				url = parameters.substr(0,startpos);
				saveas = parameters.substr(startpos +1);
				DebugMsg("DebugCommand","Download %s to %s",url.c_str(),saveas.c_str());
				HttpItem* itm = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)url.c_str(), (CHAR*)saveas.c_str());
				itm->setTag("DEBUGDOWNLOAD");
				itm->setRequester(this);
				HTTPDownloader::getInstance().AddToQueue(itm);
			}
			else
			{
				DebugMsg("DebugCommand","--Download URL SAVEAS");
			}
		}
	}
	
};