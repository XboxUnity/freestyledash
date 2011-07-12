#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


class RestartCommand:public DebugCommand
{
public :
	RestartCommand()
	{
		m_CommandName = "Restart";
	}
	void Perform(string parameters)
	{
		if(strcmp(parameters.c_str(), "system") == 0)
			HardRebootXbox(); // This will hard reboot the system and clear flash memory- good for testing plugins or dashlaunch
		else
			Restart();
	}
	
};