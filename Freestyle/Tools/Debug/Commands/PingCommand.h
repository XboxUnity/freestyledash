#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


class PingCommand:public DebugCommand
{
public :
	PingCommand()
	{
		m_CommandName = "Ping";
	}
	void Perform(string parameters)
	{
		
	
		DebugMsg("DebugCommand","Pong!");
	}
	
};