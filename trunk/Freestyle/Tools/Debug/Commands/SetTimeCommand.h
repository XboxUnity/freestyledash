#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../NTP/NTP.h"

class SetTimeCommand:public DebugCommand
{
public :
	SetTimeCommand()
	{
		m_CommandName = "SetTime";
	}
	
	void Perform(string parameters)
	{
		NTP::getInstance().SyncTime();
		DebugMsg("DebugCommand","SyncDone");
	}
	
};