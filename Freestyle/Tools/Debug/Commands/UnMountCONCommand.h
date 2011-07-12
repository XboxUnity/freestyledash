#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"

class UnMountCONCommand:public DebugCommand
{
public :
	UnMountCONCommand()
	{
		m_CommandName = "UnMount";
	}
	void Perform(string parameters)
	{
		unmountCon("con");
	
	}
	
};