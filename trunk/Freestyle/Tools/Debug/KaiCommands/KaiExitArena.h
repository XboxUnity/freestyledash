#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiExitArenaCommand:public DebugCommand
{
public :
	KaiExitArenaCommand()
	{
		m_CommandName = "KaiExitArena";
	}
	void Perform(string parameters)
	{
		KaiManager::getInstance().KaiLeaveArena();
	}
	
};