#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../SMC/smc.h"

class TrayOpenCommand:public DebugCommand
{
public :
	TrayOpenCommand()
	{
		m_CommandName = "TrayOpen";
	}
	void Perform(string parameters)
	{
		smc mySmc;
		mySmc.OpenTray();
	}
	
};