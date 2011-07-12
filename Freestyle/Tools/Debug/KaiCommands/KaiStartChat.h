#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiStartChatCommand:public DebugCommand
{
public :
	KaiStartChatCommand()
	{
		m_CommandName = "KaiStartChat";
	}
	void Perform(string parameters)
	{
		KaiManager::getInstance().KaiEnableChatmode();
	}
	
};