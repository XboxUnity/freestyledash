#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../HTTP/HTTPDownloader.h"


class HTTPPauseCommand:public DebugCommand
{
public :
	HTTPPauseCommand()
	{
		m_CommandName = "HTTPPause";
	}
	void Perform(string parameters)
	{
		HTTPDownloader::getInstance().Pause();
	}
	
};