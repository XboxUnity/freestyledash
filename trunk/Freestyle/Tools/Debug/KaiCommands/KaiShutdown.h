#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiShutdownCommand:public DebugCommand
{
public :
	KaiShutdownCommand()
	{
		m_CommandName = "KaiShutdown";
	}
	void Perform(string parameters)
	{
		KaiManager::getInstance().KaiShutdown();
	}
	
};