#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


typedef DWORD (*LoadSysModule)(char* ModulePath);

	static void RestartServer( )
	{


		int	loadStatus = 0;
		bool loaded = false;
			
//		// Resolve our function
//		LoadSysModule launchSysModule = (LoadSysModule)ResolveFunction("launch.xex", 2);
//		if(launchSysModule == NULL){
//			loadStatus = 2;
//			return;
//		}

		// Now lets launch our module
		loadStatus = XexLoadImage("game:\\Plugins\\Xbdm.xex", 8, 0, NULL);
		loaded = loadStatus == 0;

		KeSetCurrentProcessType(2);


		//		loadStatus = (int)launchSysModule("game:\\Plugins\\Xbdm.xex");
		//		loaded = loadStatus == 0;
		
		if(loaded)
			DebugMsg("Plugin", "Plugin Loaded");
		else
			DebugMsg("Plugin", "Plugin Failed");
	}

class LoadPluginCommand:public DebugCommand
{
public :
	LoadPluginCommand()
	{
		m_CommandName = "LoadPlugin";
	}
	void Perform(string parameters)
	{

		KeSetCurrentProcessType(1);

		HANDLE hThread;	DWORD threadId;
		ExCreateThread(&hThread, 0, &threadId, (VOID*)XapiThreadStartup, 
			(LPTHREAD_START_ROUTINE)RestartServer, 0, 0x02);

	}


	
};