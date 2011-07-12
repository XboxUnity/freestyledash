#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../ContentList/ContentManager.h"

class CoverCheckCommand:public DebugCommand
{
public :

	CoverCheckCommand()
	{
		m_CommandName = "CoverCheck";
	}

	void Perform(string parameters)
	{
		ContentManager::getInstance().CheckCovers();
		return;
	}
};