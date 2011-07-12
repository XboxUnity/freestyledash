#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../SMC/smc.h"

class TrayCloseCommand:public DebugCommand
{
public :
	TrayCloseCommand()
	{
		m_CommandName = "TrayClose";
	}
	void Perform(string parameters)
	{
		smc mySmc;
		mySmc.CloseTray();
	}
	
};