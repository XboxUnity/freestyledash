#pragma once
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "DebugCommand.h"

class HudCommand:public DebugCommand
{
private :
	
public :	
	HudCommand();
	
	void Perform(string parameters);
	
	
};