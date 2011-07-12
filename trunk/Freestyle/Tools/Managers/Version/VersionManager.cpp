#include <xuiresource.h>

#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "../../Settings/Settings.h"
#include "../../XEX/XeXtractor.h"
#include "../../Plugin/PluginLoader.h"

#include "VersionManager.h"

VersionManager::VersionManager()
{
	// Constructor
	DebugMsg("VersionManager", "VersionManager Activated");
	InitializeVersionData();
}

HRESULT VersionManager::InitializeVersionData()
{
	HRESULT hr = NULL;
	bool bError = false;

	hr = ExtractDashVersion();
	if(hr != S_OK)
		bError = true;

	if(FileExistsA("game:\\Skins\\Default.xzp"))
	{
		DebugMsg("VersionManager", "Extracting From Compressed Skin");
		hr = ExtractSkinVersion(true);
	}
	else
	{
		DebugMsg("VersionManager", "Extracting From Extracted Skin");
		hr = ExtractSkinVersion(false);
	}

	if(hr != S_OK)
		bError = true;
	
	hr = ExtractPluginVersion();
	if(hr != S_OK)
		bError = true;

	hr = GetKernelVersion();
	if(hr != S_OK)
		bError = true;

	return (bError == true ? S_FALSE : S_OK);
}

string VersionManager::getFSDDashVersionAsString()
{
	string Result = sprintfaA("%d.%d%s Rev%d", FSDDashApp.Version.dwMajor, FSDDashApp.Version.dwMinor, ConvertTypeToLetter(FSDDashApp.Version.dwVersionType, false).c_str(), FSDDashApp.Version.dwRevision);
	return Result;
}

string VersionManager::getFSDSkinVersionAsString()
{
	string Result = sprintfaA("%d.%d%s Rev%d", FSDDefaultSkin.Version.dwMajor, FSDDefaultSkin.Version.dwMinor, ConvertTypeToLetter(FSDDefaultSkin.Version.dwVersionType, false).c_str(), FSDDefaultSkin.Version.dwRevision);
	return Result;
}

string VersionManager::getFSDPluginVersionAsString()
{
	string Result = sprintfaA("%d.%d%s Rev%d", FSDPluginApp.Version.dwMajor, FSDPluginApp.Version.dwMinor, ConvertTypeToLetter(FSDPluginApp.Version.dwVersionType, false).c_str(), FSDPluginApp.Version.dwRevision);
	return Result;
}

string VersionManager::getKernelAsString()
{
	string Result = sprintfaA("%d.%d.%d.%d", Kernel.Major, Kernel.Minor, Kernel.Build, Kernel.QFE);
	return Result;
}

HRESULT VersionManager::ReadVersionFromFile(string szFilePath, PVERSION_INFO pVersionInfo)
{
	// Extract Data from file 
	FILE * fHandle = NULL;

	DebugMsg("VersionManager", "Read Version File:  %s", szFilePath.c_str());
	fopen_s(&fHandle, szFilePath.c_str(), "rb");
	fread(pVersionInfo, sizeof(VERSION_INFO), 1, fHandle);
	fclose(fHandle);

	return S_OK;
}

HRESULT VersionManager::WriteVersionToFile(string szFilePath, VERSION_INFO versionInfo)
{
	FILE * fHandle = NULL;

	DebugMsg("VersionManager", "Write Version File:  %s", szFilePath.c_str());
	fopen_s(&fHandle, szFilePath.c_str(), "wb");
	fwrite(&versionInfo, sizeof(VERSION_INFO), 1, fHandle);
	fclose(fHandle);

	return S_OK;
}

string VersionManager::ConvertTypeToLetter( int nVersionType, bool useUpperCase = false )
{
	string retval;

	switch (nVersionType)
	{
	case 1:
		if(useUpperCase)
			retval = "A";
		else
			retval = "a";
		break;
	case 2:
		if(useUpperCase)
			retval = "B";
		else
			retval = "b";
		break;
	case 0:
	case 3:
		retval = "";
		break;
	};

	return retval;

}

wstring VersionManager::ConvertTypeToLetterW( int nVersionType, bool useUpperCase = false )
{
	wstring retval;

	switch (nVersionType)
	{
	case 1:
		if(useUpperCase)
			retval = L"A";
		else
			retval = L"a";
		break;
	case 2:
		if(useUpperCase)
			retval = L"B";
		else
			retval = L"b";
		break;
	case 0:
	case 3:
		retval = L"";
		break;
	};

	return retval;

}

string VersionManager::ConvertTypeToString( int nVersionType, bool addSeperator = false )
{
	string retval;

	switch (nVersionType)
	{
	case 1:
		if(addSeperator)
			retval = "- Alpha";
		else
			retval = "Alpha";
		break;
	case 2:
		if(addSeperator)
			retval = "- Beta";
		else
			retval = "Beta";
		break;
	case 0:
	case 3:
		retval = "";
		break;
	};

	return retval;
}

wstring VersionManager::ConvertTypeToStringW( int nVersionType, bool addSeperator = false )
{
	wstring retval;

	switch (nVersionType) {
	case 1:
		if(addSeperator)
			retval = L"- Alpha";
		else
			retval = L"Alpha";
		break;
	case 2:
		if(addSeperator)
			retval = L"- Beta";
		else
			retval = L"Beta";
		break;
	case 0:
	case 3:
		retval = L"";
		break;
	};

	return retval;
}

BOOL VersionManager::Compare(VERSION_DATA newVersion, VERSION_DATA curVersion, BOOL bIncludeBeta)
{
	// Check to see if the received major version is larger than the current
	if(newVersion.dwMajor > curVersion.dwMajor) return TRUE;
	
	if(newVersion.dwMajor == curVersion.dwMajor) {
		if(newVersion.dwMinor > curVersion.dwMinor) return TRUE;

		if(bIncludeBeta) {
			if(newVersion.dwRevision > curVersion.dwRevision ) return TRUE;
		}
	}

	// Incoming VERSION_DATA is not larger than current VERSION_DATA
	return FALSE;
}

BOOL VersionManager::IsVersionDataEqual( VERSION_DATA versionA, VERSION_DATA versionB ) {
	// Compare Version Data and return results
	int ret = memcmp(&versionA, &versionB, sizeof(VERSION_INFO));
	return ret == 0 ? TRUE : FALSE;
}

BOOL VersionManager::IsVersionWithinRange( VERSION_DATA checkVersion, VERSION_DATA maxVersion, VERSION_DATA minVersion)
{
	BOOL bInRange = FALSE;
	BOOL isAboveMin = Compare(checkVersion, minVersion, true);
	BOOL isBelowMax = !Compare(checkVersion, maxVersion, true);

	if(isBelowMax == FALSE) isBelowMax = IsVersionDataEqual(checkVersion, maxVersion);
	if(isAboveMin == FALSE) isAboveMin = IsVersionDataEqual(checkVersion, minVersion);
	if(isBelowMax && isAboveMin) bInRange = TRUE;
	
	return bInRange;
}

BOOL VersionManager::IsVersionInfoEqual( VERSION_INFO versionA, VERSION_INFO versionB, BOOL includeRange, BOOL rangeOnly ) {
	if(rangeOnly == FALSE) {
		if(IsVersionDataEqual(versionA.Version, versionB.Version) == FALSE) return FALSE;
	} else includeRange = TRUE;
	if(includeRange == TRUE) {
		if(IsVersionDataEqual(versionA.Minimum, versionB.Minimum) == FALSE) return FALSE;
		if(IsVersionDataEqual(versionA.Maximum, versionB.Maximum) == FALSE) return FALSE;
	}
	return TRUE;
}

BOOL VersionManager::IsVersionInfoNull( VERSION_INFO versionInfo ) {
	VERSION_INFO nullVersionInfo;
	CreateNullVersionInfo(&nullVersionInfo);
	return IsVersionInfoEqual(versionInfo, nullVersionInfo, true, false);
}

HRESULT VersionManager::CreateNullVersionInfo( PVERSION_INFO pVersionInfo ) {
	// NULL the incoming VERSION_INFO struct
	memset(pVersionInfo, 0, sizeof(VERSION_INFO));
	return S_OK;
}

HRESULT VersionManager::ExtractDashVersion( void )
{
	VOID * pSectionData;
	DWORD dwSectionSize;

	// Extract the embedded version file within our xex to memory
	if(XGetModuleSection( GetModuleHandle(NULL), "FSDVer", &pSectionData, &dwSectionSize) == FALSE) {
		DebugMsg("VersionManager", "There was an error extracting Dash Version Information.  Loading NULL verison data");
		CreateNullVersionInfo(&FSDDashApp);
		return S_FALSE;
	}

	// Copy our data to the FSDDashApp struct
	memset(&FSDDashApp, 0, sizeof(VERSION_INFO));
	memcpy(&FSDDashApp, (VERSION_INFO*)pSectionData, sizeof(VERSION_INFO));
	return S_OK;
}

HRESULT VersionManager::ExtractSkinVersion( BOOL isCompressed )
{
	HXUIPACKAGE phPackage;

	// Predefine some path information
	const WCHAR szSkinPathXZP[] = L"file://game:\\skins\\default.xzp";
	const char szSkinPath[] = "game:\\skins\\default\\";
	const WCHAR szVersionFileW[] = L"skin.ver";
	const char szVersionFile[] = "skin.ver";

	if(isCompressed == TRUE) {
		// Open up the XZP file for reading
		if( XuiResourceOpenPackage((WCHAR*)szSkinPathXZP, &phPackage, false) != S_OK ) {
			DebugMsg("VersionManager", "Failed to open XZP File; Skin Version extract failed.");
			CreateNullVersionInfo(&FSDDefaultSkin);
			return S_FALSE;
		}
		// Define a resource locator string
		wstring szLocator = sprintfaW(L"%s#%s", szSkinPathXZP, szVersionFileW);
		
		BYTE * pSectionData;
		UINT pSectionSize;
		
		// Extract the embedded version file within our skin xzp
		if( XuiResourceLoadAllNoLoc(szLocator.c_str(), &pSectionData, &pSectionSize) != S_OK ) {
			DebugMsg("VersionManager", "Failed to extract SkinVersion Information From XZP");
			CreateNullVersionInfo(&FSDDefaultSkin);
			return S_FALSE;
		}

		// Copy our data to the FSDDefaultSkin struct
		memset(&FSDDefaultSkin, 0, sizeof(VERSION_INFO));
		memcpy(&FSDDefaultSkin, (VERSION_INFO*)pSectionData, sizeof(VERSION_INFO));

		// Free our Resource Data and release our package
		XuiFree((VOID*)pSectionData);
		XuiResourceReleasePackage(phPackage);

	} else {
		string szFilePath = sprintfaA("%s%s", szSkinPath,szVersionFile);
		if(!FileExistsA(szFilePath)) {
			DebugMsg("VersionManager", "Failed to extract Skin Version Information from Extracted Skin");
			CreateNullVersionInfo(&FSDDefaultSkin);
			return S_FALSE;
		}
		// Read the .ver binary file into our VERSION_INFO struct
		ReadVersionFromFile(szFilePath, &FSDDefaultSkin);
	}

	// Return successfully
	return S_OK;
}

HRESULT VersionManager::ExtractPluginVersion( void )
{
	string szPluginPath = PLUGIN_PATH;
	if(!FileExistsA(szPluginPath)) {
		DebugMsg("Version Manager", "Failed to extract Plugin Version Information");
		CreateNullVersionInfo(&FSDPluginApp);
		return S_FALSE;
	}
	HRESULT hr = NULL;

	// Plugin is XEX file so we will extract the embedded version file from it
	XeXtractor pluginFile;
	PBYTE pSectionData;
	DWORD dwSectionSize;

	// Open the xex and load into memory
	if( pluginFile.OpenXex(szPluginPath) != S_OK ) {
		DebugMsg("Version Manager", "Failed to extract Plugin Version Information");
		CreateNullVersionInfo(&FSDPluginApp);
		return S_FALSE;
	}
	
	// Extract the resource from the decrypted xex file
	if( pluginFile.ExtractResource("PlugVer", &pSectionData, &dwSectionSize) != S_OK ) {
		pluginFile.CloseXex();
		DebugMsg("Version Manager", "Failed to extract Plugin Version Information");
		CreateNullVersionInfo(&FSDPluginApp);
		return S_FALSE;	
	}

	// Copy our data to the FSDPluginApp struct
	memset(&FSDPluginApp, 0, sizeof(VERSION_INFO));
	memcpy(&FSDPluginApp, (VERSION_INFO*)pSectionData, sizeof(VERSION_INFO));
	
	// Close open xex and return successfully
	pluginFile.CloseXex();
	return S_OK;
}

HRESULT VersionManager::GetKernelVersion() 
{
	// Copy our kernel version
	memcpy(&Kernel, XboxKrnlVersion, sizeof(VERSION_KERNEL));
	return S_OK;
}