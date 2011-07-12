#pragma once
#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"

class DebugCommand
{
protected :
	string m_CommandName;
public :
	virtual void Perform(string parameters)=0;

	string getCommandName()
	{
		return m_CommandName;
	}
};

