#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../HTTP/HTTPDownloader.h"


class HTTPResumeCommand:public DebugCommand
{
public :
	HTTPResumeCommand()
	{
		m_CommandName = "HTTPResume";
	}
	void Perform(string parameters)
	{
		HTTPDownloader::getInstance().Resume();
	}
	
};