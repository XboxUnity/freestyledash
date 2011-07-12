#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiEndChatCommand:public DebugCommand
{
public :
	KaiEndChatCommand()
	{
		m_CommandName = "KaiEndChat";
	}
	void Perform(string parameters)
	{
		KaiManager::getInstance().KaiLeaveChatmode();
	}
	
};