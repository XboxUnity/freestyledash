#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


class VirtualKeyCommand:public DebugCommand
{
public :
	struct Client {
		int test;
	};	
	VirtualKeyCommand()
	{
		m_CommandName = "VKey";
	}
	void Perform(string parameters)
	{
		WORD keyPress = StringToVKey(parameters);

		XEnableScreenSaver(false);

		if(keyPress > 0){
			SendXuiKeyStroke(keyPress);
			DebugMsg("VirtualKey", "%s Pressed.", parameters.c_str());
		}
		else
		{
			DebugMsg("VirtualKey", "Invalid Key Press Parameter");
		}
	}

};