#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"






class GetTimeCommand:public DebugCommand
{
public :
	GetTimeCommand()
	{
		m_CommandName = "GetTime";
	}
	void Perform(string parameters)
	{
		  SYSTEMTIME LocalSysTime;
		GetLocalTime( &LocalSysTime );
		DebugMsg("DebugCommand","Struct Size : %d",sizeof(LocalSysTime));
		DebugMsg("DebugCommand","Hours : %d",LocalSysTime.wHour);
		DebugMsg("DebugCommand","Minutes : %d",LocalSysTime.wMinute);
		DebugMsg("DebugCommand","Seconds : %d",LocalSysTime.wSecond);
		DebugMsg("DebugCommand","Milliseconds : %d",LocalSysTime.wMilliseconds);
		DebugMsg("DebugCommand","Day : %d",LocalSysTime.wDay);
		DebugMsg("DebugCommand","Day of week : %d",LocalSysTime.wDayOfWeek);
		DebugMsg("DebugCommand","Month : %d",LocalSysTime.wMonth);
		DebugMsg("DebugCommand","Year : %d",LocalSysTime.wYear);
		
	}
	
};