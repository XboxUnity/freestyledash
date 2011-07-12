#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "../Commands/DebugCommand.h"
#include "../../XlinkKai/General/KaiManager.h"


class KaiJoinArenaCommand:public DebugCommand
{
public :
	KaiJoinArenaCommand()
	{
		m_CommandName = "KaiJoinArena";
	}
	void Perform(string parameters)
	{
		vector<string> args;
		StringSplit(parameters, ";", &args);
		
		if(args.size() == 0)
		{
			DebugMsg("KaiJoinArenaCommand", "Usage #1:  KaiJoinArena Vector");
			DebugMsg("KaiJoinArenaCommand", "Usage #2:  KaiJoinArena Vector Password");
		}
		else if(args.size() == 1)
		{
			DebugMsg("KaiJoinArenaCommand", "Vector:  '%s',  Password:  'None'", args.at(0).c_str());

			string szArena = args.at(0);
			string pass = "";
			KaiManager::getInstance().KaiEnterArena(szArena, pass);
		}		
		else if(args.size() == 2)
		{
			DebugMsg("KaiJoinArenaCommand", "Vector:  '%s',  Password:  '%s'; '%s'", args.at(0).c_str(), args.at(1).c_str());

			string szArena = args.at(0).c_str();
			string pass = args.at(1).c_str();
			KaiManager::getInstance().KaiEnterArena(szArena, pass);
		}	
	}
	
};