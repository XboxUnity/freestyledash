#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiPMCommand:public DebugCommand
{
public :
	KaiPMCommand()
	{
		m_CommandName = "KaiPM";
	}
	void Perform(string parameters)
	{
		vector<string> args;
		StringSplit(parameters, " ", &args);
		
		if(args.size() == 0)
		{
		}
		else if(args.size() == 1)
		{
		}		
		else
		{
			DebugMsg("KaiPMCommand", "Player:  '%s',  Message:  '%s'", args.at(0).c_str(), args.at(1).c_str());
			KaiManager::getInstance().SendPrivateMessage(args.at(0).c_str(), args.at(1).c_str());
		}	
	}
	
};