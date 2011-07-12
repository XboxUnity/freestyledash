#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiConnectCommand:public DebugCommand
{
public :
	KaiConnectCommand()
	{
		m_CommandName = "KaiConnect";
	}
	void Perform(string parameters)
	{
		KaiManager::getInstance().KaiAttachToClient();
	}
	
};