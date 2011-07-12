#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"


class ToggleDebugDisplay:public DebugCommand
{
public :

	ToggleDebugDisplay()
	{
		m_CommandName = "ToggleDebugDisplay";
	}

	void Perform(string parameters)
	{
		bool ToggleDisplay = IsDebugMode();

		SetDebugDisplay(!ToggleDisplay);
	}

};