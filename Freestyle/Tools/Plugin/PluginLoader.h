#pragma once
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Managers/Version/VersionManager.h"

#define PLUGIN_PATH "game:\\Plugins\\FreeStyleDashPlugin.xex"

typedef struct _MULTIDISC_INFO {
	DWORD TitleID;
	BYTE  CurrentDiscNum;
	BYTE  DiscsInSet;
	BOOL  DiscIsCon[5];
	CHAR  DiscPath[5][MAX_PATH];
} MULTIDISC_INFO, *PMULTIDISC_INFO;

typedef HRESULT (*pEnableTrainer)(CHAR* TrainerPath);
typedef HRESULT (*pEnableMultiDisc)(MULTIDISC_INFO* MultiDiscInfo);
typedef HRESULT (*pSetScreenshotPath)(const CHAR* Path);

class PluginLoader {
private:

public:	

	VERSION_INFO     vPluginVersion;
	pEnableTrainer   EnableTrainer;
	pEnableMultiDisc EnableMultiDisc;
	pSetScreenshotPath SetScreenshotPath;

	DWORD        m_dwLoadStatus;
	BOOL         m_bIsLoaded;

	static PluginLoader& getInstance() {
		static PluginLoader singleton;
		return singleton;
	}

	DWORD GetStatus()               { return m_dwLoadStatus; }
	BOOL IsLoaded()                 { return m_bIsLoaded;    }
	VERSION_INFO GetPluginVersion() { return vPluginVersion; }

	PluginLoader();
	~PluginLoader();
	PluginLoader(const PluginLoader&);
	PluginLoader& operator=(const PluginLoader&);

	HRESULT Initialize();
	void ResolveImports();
};