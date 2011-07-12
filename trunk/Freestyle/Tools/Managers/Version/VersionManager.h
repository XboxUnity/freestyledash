#pragma once
#include "../../Generic/KernelExports.h"

#define VERSION_TYPE_UNDEFINED 0
#define VERSION_TYPE_ALPHA     1
#define VERSION_TYPE_BETA      2
#define VERSION_TYPE_RELEASE   3

typedef struct _VERSION_DATA {
	DWORD dwMajor;
	DWORD dwMinor;
	DWORD dwRevision;
	DWORD dwVersionType; 
} VERSION_DATA, *PVERSION_DATA;

typedef struct VERSION_INFO {
	VERSION_DATA Version;
	VERSION_DATA Minimum;
	VERSION_DATA Maximum;
} VERSION_INFO, *PVERSION_INFO;

class VersionManager
{
public:
	static VersionManager& getInstance()
	{
		static VersionManager singleton;
		return singleton;
	}

	HRESULT ReadVersionFromFile(string szFilePath, PVERSION_INFO pVersionInfo);
	//VersionPack ReadVersionFromFile(string szFilePath);
	HRESULT WriteVersionToFile(string szFilePath, VERSION_INFO versionData);
	//void WriteVersionToFile(string szFilePath, VersionPack versionData);
	HRESULT CreateNullVersionInfo(PVERSION_INFO pVersionInfo);
	//VersionPack CreateNullVersionPack( void );

	// Conversion Functions
	string ConvertTypeToString( int nVersionType, bool addSeperator );
	wstring ConvertTypeToStringW( int nVersionType, bool addSeperator );
	string ConvertTypeToLetter( int nVersionType, bool useUpperCase );
	wstring ConvertTypeToLetterW( int nVersionType, bool useUpperCase );

	// Comparison Functions
	BOOL IsVersionDataEqual( VERSION_DATA versionA, VERSION_DATA versionB );
	BOOL IsVersionInfoEqual( VERSION_INFO versionA, VERSION_INFO versionB, BOOL includeRange, BOOL rangeOnly );
	BOOL IsVersionWithinRange( VERSION_DATA checkVersion, VERSION_DATA maxVersion, VERSION_DATA minVersion);
	BOOL IsVersionInfoNull( VERSION_INFO version );
	BOOL Compare( VERSION_DATA newVersion, VERSION_DATA curVersion, BOOL bIncludeBeta );

	VERSION_INFO getFSDDashVersion() { return FSDDashApp; }
	VERSION_INFO getFSDSkinVersion() { return FSDDefaultSkin; }
	VERSION_INFO getFSDPluginVersion() { return FSDPluginApp; }
	VERSION_KERNEL getKernelVersion() { return Kernel; }

	HRESULT ExtractDashVersion( void );
	HRESULT ExtractSkinVersion( BOOL isCompressed );
	HRESULT ExtractPluginVersion( void );
	HRESULT GetKernelVersion( void );

	string getFSDDashVersionAsString();
	string getFSDSkinVersionAsString();
	string getFSDPluginVersionAsString();
	string getKernelAsString();

private:

	VERSION_INFO FSDDashApp;
	VERSION_INFO FSDPluginApp;
	VERSION_INFO FSDDefaultSkin;
	VERSION_KERNEL Kernel;
	HRESULT InitializeVersionData();
	
	VersionManager();
	~VersionManager(){}
	VersionManager(const VersionManager&);                 // Prevent copy-construction
	VersionManager& operator=(const VersionManager&);      // Prevent assignment

};