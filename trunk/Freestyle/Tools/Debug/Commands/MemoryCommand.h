#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


class MemoryCommand:public DebugCommand
{
public :
	MemoryCommand()
	{
		m_CommandName = "Memory";
	}
	void Perform(string parameters)
	{
		MEMORYSTATUS stat;
		GlobalMemoryStatus(&stat);

		size_t avail = stat.dwAvailPhys;
		size_t total = stat.dwTotalPhys;
		size_t used = total - avail;

		DebugMsg("DebugCommand", "Current Memory Statistics:");
		DebugMsg("DebugCommand", "\t\tAvailable: %d bytes", avail);
		DebugMsg("DebugCommand", "\t\tUtilized: %d bytes", used);
		DebugMsg("DebugCommand", "\t\tTotal: %d bytes", total);
	}
	
};