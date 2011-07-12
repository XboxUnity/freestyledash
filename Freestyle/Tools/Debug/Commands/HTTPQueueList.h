#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../../HTTP/HTTPDownloader.h"
#include "DebugCommand.h"


class HTTPQueueListCommand:public DebugCommand
{
public :
	HTTPQueueListCommand()
	{
		m_CommandName = "HTTPQueueList";
	}
	void Perform(string parameters)
	{
		
		HTTPDownloader::getInstance().DebugListDownloadQueue();
	//	DebugMsg("HTTPQueueList","Pong!");
	}
	
};