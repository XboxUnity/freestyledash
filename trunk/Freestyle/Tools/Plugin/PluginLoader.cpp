#include "stdafx.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"

#include "PluginLoader.h"

static void LoadPluginThread() {

	// Lets launch our module
	DWORD loadStatus = XexLoadImage(PLUGIN_PATH, 8, 0, NULL);

	// Set our load status
	PluginLoader::getInstance().m_dwLoadStatus = loadStatus;
	PluginLoader::getInstance().m_bIsLoaded	   = loadStatus == 0;

	// Resolve if we are loaded
	if(PluginLoader::getInstance().m_bIsLoaded)
		PluginLoader::getInstance().ResolveImports();
}

PluginLoader::PluginLoader() {
	
	// Constructor
	m_dwLoadStatus = 0;
	m_bIsLoaded = FALSE;
}

PluginLoader::~PluginLoader() {
	
	// Deconstructor
}

HRESULT PluginLoader::Initialize( void ) {

	DebugMsg("PluginLoader", "Initializing Plugin");

	// If we are not loaded lets load it
	if(GetModuleHandle("FreeStyleDashPlugin.xex") == NULL) {
		
		HANDLE hThread;	DWORD threadId;
		ExCreateThread(&hThread, 0, &threadId, (VOID*)XapiThreadStartup, 
				(LPTHREAD_START_ROUTINE)LoadPluginThread, NULL, 0x02);
		XSetThreadProcessor(hThread, 4);
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
		ResumeThread(hThread);
	}
	else {
		m_bIsLoaded = TRUE;
		ResolveImports();
	}

	return S_OK;
}
void PluginLoader::ResolveImports() {

	// Get our module handle first
	HMODULE mHandle = GetModuleHandle("FreeStyleDashPlugin.xex");
	if(mHandle == NULL) {
		m_bIsLoaded = FALSE;
		return;
	}

	// Now we can start resolving things
	vPluginVersion  = *(VERSION_INFO*)GetProcAddress(mHandle, (LPCSTR)2);
	EnableTrainer   = (pEnableTrainer)GetProcAddress(mHandle, (LPCSTR)3);
	EnableMultiDisc = (pEnableMultiDisc)GetProcAddress(mHandle, (LPCSTR)4);
	SetScreenshotPath = (pSetScreenshotPath)GetProcAddress(mHandle, (LPCSTR)5);

}