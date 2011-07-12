#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


class LaunchXEXCommand:public DebugCommand
{
public :
	LaunchXEXCommand()
	{
		m_CommandName = "LaunchXEX";
	}
	void Perform(string parameters)
	{
		/*XLaunchNewImage("game:\\FreestyleDash.xex",0);	*/
		DebugMsg("Params :%s",parameters.c_str());
		if(parameters.length() == 0)
		{
			DebugMsg("DebugCommand"," -- XexPath");
			return;
		}
		else
		{
		if(FileExistsA(parameters))
		{
			XLaunchNewImage(parameters.c_str(),0);
		}
		}

	}
	
};