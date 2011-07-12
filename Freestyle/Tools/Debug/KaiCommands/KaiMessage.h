#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiMessageCommand:public DebugCommand
{
public :
	KaiMessageCommand()
	{
		m_CommandName = "KaiMessage";
	}
	void Perform(string parameters)
	{
		KaiManager::getInstance().KaiSendMessage(parameters);
	}
	
};