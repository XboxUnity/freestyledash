#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "HudCommand.h"

#include "../../Managers/Skin/SkinManager.h"

#include "../../../Application/FreestyleApp.h"
#include "../../Generic/xboxtools.h"
#include "../../Generic/PNG/lodepng.h"
#include "../../Texture/TextureCache.h"
#include "../../XEX/Xbox360Container.h"
#include "../../Generic/DDS/DDSTools.h"

#ifdef _DEBUG
#include <xbdm.h>
#pragma comment(lib, "xbdm.lib")
#endif

#define XAM_LOAD_ADDRESS 0x817E0000
#define APP_LOAD_ADDRESS 0x82000000
#define HUD_LOAD_ADDRESS 0x913E0000
#define DASH_LOAD_ADDRESS 0x92000000

HRESULT CreateSymbolicLink(CHAR* szDrive, CHAR* szDeviceName, BOOL System) {

	// Setup our path
	CHAR szDestinationDrive[MAX_PATH];
	if(System)
		sprintf_s(szDestinationDrive, MAX_PATH, "\\System??\\%s", szDrive);
	else
		sprintf_s(szDestinationDrive, MAX_PATH, "\\??\\%s", szDrive);

	// Setup our strings
	STRING linkname, devicename;
	RtlInitAnsiString(&linkname, szDestinationDrive);
	RtlInitAnsiString(&devicename, szDeviceName);

	// Create finally
	NTSTATUS status = ObCreateSymbolicLink(&linkname, &devicename);
	if (status >= 0)
		return S_OK;
	return S_FALSE;
}

typedef HRESULT (*TestFunc)(LPCWSTR, LPCWSTR, void*, HXUIOBJ*);

HRESULT XuiSceneCreateHook(LPCWSTR szBasePath, LPCWSTR szScenePath, void* pvInitData, HXUIOBJ* phScene) {
	
	WCHAR* szPath = L"file://game:/\0";
	WCHAR* szFile = L"options.xur\0";
	
	WCHAR* szPath2 = L"file://game:/\0";
	WCHAR* szFile2 = L"custom.xur\0";

	//WCHAR* szFile = L"QuickLaunch.xur";
	// Continue to our normal function
//	size_t len = wcslen(szBasePath);
//	memset((void*)szBasePath, 0, len * 2);
//	wcscpy((wchar_t*)szBasePath, szPath);
//	len = wcslen(szScenePath);
//	memset((void*)szScenePath, 0, len * 2);
//	wcscpy((wchar_t*)szScenePath, szFile);

	TestFunc myFunc = (TestFunc)0x81AECEC8;
	return myFunc(szPath, szFile, pvInitData, phScene);
	//return XuiSceneCreate(szPath2, szFile2, pvInitData, phScene);
}

void PatchXuiSceneCreateInHud()
{
#ifdef _DEBUG
	UINT32 data[4];
	PatchInJump(data, (UINT32)XuiSceneCreateHook, FALSE);
// (UINT32*)

	DWORD hudAddress = HUD_LOAD_ADDRESS + 0x212CC;

	DmSetMemory((LPVOID)hudAddress, 16, data, NULL); 
	CreateSymbolicLink("hdd1:", "\\Device\\Harddisk0\\Partition1", TRUE);
#endif
}

HudCommand::HudCommand()
{
	m_CommandName = "Hud";
}

void MyHook(void)
{

	return;
}
typedef long (*OnXenonButtonSpoof)(void*);

void HudCommand::Perform(string parameters)
{
	if(parameters == "1")
		ForcePlayerSignIn(0, 0xE000158C485D6E42 );
	else if( parameters == "2" )
		ForcePlayerSignIn(0, 0xE0001682485D6E42 );
	else if ( parameters == "3" ) {
		ForcePlayerSignOut(0);
		ForcePlayerSignOut(1);
		ForcePlayerSignOut(2);
		ForcePlayerSignOut(3);
	}
	else if( parameters == "4" )
		ForcePlayerSignIn(1, 0xE000158C485D6E42 );
	else if( parameters == "5" )
		ForcePlayerSignIn(1, 0xE0001682485D6E42 );


/*
	if(parameters == "1")
	{
		
		//PatchXuiSceneCreateInHud();
	}
	else if(parameters == "2")
	{
		//HXUIOBJ hScene;
		//XuiSceneCreate(L"file://game:/xui/", sprintfaW(L"xui%d.xur", x).c_str(), NULL, &hScene);
	}
	else
	{

	}*/
}
	
