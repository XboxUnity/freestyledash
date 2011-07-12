#pragma once
#include "AtgConsole.h"
#include "AtgUtil.h"
#include "AtgInput.h"
//#include "../../Application/FreestyleApp.h"
#include "../HTTP/HttpItem.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Settings/Settings.h"
#include "../HTTP/HTTPDownloader.h"
#include "../XML/XMLReader.h"
#include "../Managers/XZP/XZPManager.h"

#define VERSION_UNDEFINED	0x00
#define VERSION_ALPHA		0x01
#define VERSION_BETA		0x02
#define VERSION_RELEASE		0x03
#define VERSION_CHANGELOG   0x04

#define FILETYPE_OTHER		0x00
#define FILETYPE_DASH		0x01
#define FILETYPE_SKIN		0x02
#define FILETYPE_PLUGIN		0x03

static WCHAR updStringA[] = L"New Dash Update Available";
static WCHAR updStringB[] = L"New Skin Update Available";
static WCHAR updStringC[] = L"New Plug-In Update Available";
static WCHAR updStringD[] = L"New Updates Available";

typedef struct
{
	int nMajorVersion;
	int nMinorVersion;
	int nRevisionNumber;
	int nVersionType;
	int nFileType;
	int nUpdateID;
	string szFileName;
	string szName;
} VersionDescriptor;

typedef struct
{
	string szName;
	string szDate;
	string szLogData;

} ChangelogDescriptor;

typedef struct
{
	string defName;
	string updDate;
} DocumentHeader;

typedef struct
{
	int dashTotal;
	int skinTotal;
	int pluginTotal;
} UpdateHeader;

typedef struct
{
	DocumentHeader docHeader;
	UpdateHeader updHeader;
	string szFilePath;
} Header;

typedef struct
{
	Header xmlHeader;
	map<int, VersionDescriptor> xmlDescriptors;
	ChangelogDescriptor xmlChangeLog;
} XmlDocument;

typedef struct
{
	int nFileType;
	int nListIndex;
	int nMapIndex;
	string szDisplayTitles;
} ListItem;

typedef struct
{
	int nListItemCount;
	vector<ListItem> listItems;
} ListGroup;

typedef struct
{
	int nMapIndex;
	int nListIndex;
} DownloadPacket;

class iUpdateDisplayer{
public :
	virtual void handleStatusUpdate( string szUpdate ) = 0;
	virtual void handleDownloadFinished( int nItem ) = 0;
	virtual void handleLoadingComplete( void ) = 0;
};

class AutoUpdater : public iHttpItemRequester
{
public:
	void DownloadCompleted(HttpItem * itm );
	HRESULT DownloadQueuedItems( ListGroup dashQueue, ListGroup skinQueue, ListGroup pluginQueue);
	HRESULT CheckForUpdates( bool notifyUser );
	ListGroup getListMap(int FileType);
	void RunInstaller( void );

	ChangelogDescriptor * getChangelogDescriptor( void ) { return &(xmlDoc.xmlChangeLog); }

	// Functions to add and remove yourself from Observer List
	void add(iUpdateDisplayer& ref);
	void remove(iUpdateDisplayer& ref);

	static AutoUpdater& getInstance()
	{
		static AutoUpdater singleton;
		return singleton;
	}

private :
	// Privte Variables
	XmlDocument xmlDoc;
	map<HttpItem*, DownloadPacket> m_dlRequest;
	
	ListGroup CreateDashListMap( bool bIncludeBetaBuilds );
	ListGroup CreateSkinListMap( bool bIncludeBetaBuilds );
	ListGroup CreatePluginListMap( bool bIncludeBetaBuilds );
	void RestartDashboard(ATG::Console * console, bool resetFlag);
	void PromptToRestart();
	
	bool InstallDashUpdate(ATG::Console * console, string szDestination);
	bool InstallSkinUpdate(ATG::Console * console, string szDestination);
	bool InstallPluginUpdate(ATG::Console * console, string szDestination);

	ListGroup dashList;
	ListGroup skinList;
	ListGroup pluginList;

	// Private Functions
	HRESULT ParseCurrentVersionXML(HttpItem *itm);
	HRESULT DownloadCurrentXML( void );
	HRESULT ReadSettings( void );
	HRESULT ClearTempFiles( void );
	HRESULT ClearXMLDocumentData( void );
	string ConvertTypeToString( int nType );

	string getAutoupdaterXmlPath();
	HttpItem* getAutoupdateXmlHttpItem();
	HttpItem* getAutoupdateXeXHttpItem(string fileName);
	HttpItem* getAutoupdateSkinHttpItem(string fileName);
	HttpItem* getAutoupdatePluginHttpItem(string fileName);
	HttpItem* getAutoupdateChangeLogHttpItem(string fileName);
	string getAutoupdaterTempXexVerPath(bool erase);
	string getAutoupdaterTempXexPath(bool erase);
	string getAutoupdaterTempSkinVerPath(bool erase);
	string getAutoupdaterTempSkinPath(bool erase);
	string getAutoupdaterTempPluginVerPath(bool erase);
	string getAutoupdaterTempPluginPath(bool erase);
	string getAutoupdaterTempChangeLogPath(bool erase);

	void HandleUserNotification( void );
	void AutoUpdater::InitializeInstaller( void );
	bool VersionCompare(VersionDescriptor newVersion, VersionDescriptor curVersion, bool bIncludeRevs, bool bReleaseBuildOnly);

	// Settings Variables
	bool bUpdatedXexNotice;
	bool bUpdatedSkinNotice;
	bool bUpdatedPluginNotice;
	bool bUpdatedBetaNotice;

	bool bErrorOccurred;

	bool m_StateNotifyUser;
	bool m_StateListsLoaded;
	bool m_StateDownloading;

	VersionDescriptor curDashVersion;
	VersionDescriptor curSkinVersion;
	VersionDescriptor curPluginVersion;
	VersionDescriptor * tempVersion;
	ChangelogDescriptor * tempChangelog;

	//iUpdateDisplayer Maps
	std::map<iUpdateDisplayer* const,iUpdateDisplayer* const> _observers;
	typedef std::map<iUpdateDisplayer* const, iUpdateDisplayer* const> item;

	//iUpdateDisplayer Implementation
	void _notifyStatusUpdate(string szStatus);
	void _notifyDownloadFinished( int nItem );
	void _notifyLoadingComplete( void );
	
	// Constructor - Deconstructor
	AutoUpdater();
	~AutoUpdater() {}
    AutoUpdater(const AutoUpdater&);                 // Prevent copy-construction
    AutoUpdater& operator=(const AutoUpdater&);      // Prevent assignment
};