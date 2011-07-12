 #pragma once
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../XML/xmlreader.h"
#include "../Debug/Debug.h"
#include "../Managers/VariablesCache/VariablesCache.h"
#include "../Managers/Version/VersionManager.h"
#include "../Managers/RSS/RssReader.h"

struct SortIds
{
	SortIds(int s, int d, int v) : sortOrder(s), sortDirection(d), subtitleView(v) {};
	int sortOrder;
	int sortDirection;
	int subtitleView;
};

struct ScanPath
{
	ScanPath() : PathId(0), Path(""), RetailTab(""), DevkitTab(""), RetailTabId(0), DevkitTabId(0), ScanDepth(0) {};
	ScanPath(ULONGLONG id, string Path, string RetailTab, string DevkitTab, ULONGLONG RetailTabId, ULONGLONG DevkitTabId, int ScanDepth) 
		: PathId(id), Path(Path), RetailTab(RetailTab), DevkitTab(DevkitTab), RetailTabId(RetailTabId), DevkitTabId(DevkitTabId), ScanDepth(ScanDepth) {};
	ULONGLONG PathId;
	string Path;
	string RetailTab;
	string DevkitTab;
	ULONGLONG RetailTabId;
	ULONGLONG DevkitTabId;
	int ScanDepth;
};

class SETTINGS
{
public:
	static SETTINGS& getInstance()
	{
		static SETTINGS singleton;
		return singleton;
	}
	
	// this is crap, get rid of it when we start using TabIds
	ULONGLONG InternalTabNameToDatabaseTabId(string name);
	string DatabaseTabIdToInternalTabName(ULONGLONG tabId);

	int getDisableAutoScan() { return m_DisableAutoScan; }
	void setDisableAutoScan(int value ) { m_DisableAutoScan = value; Write(); }

	int getResumeAssetDownload() { return m_ResumeAssetDownload; }
	void setResumeAssetDownload( int value ) { m_ResumeAssetDownload = value; Write(); }

	string getDataPath() { return m_DataPath; }
	void setDataPath(string DataPath, bool writeData = true);

	string getDataPathX() {	return m_DataPathX;	}
	void setDataPathX(string DataPath) { m_DataPathX = DataPath; /* Write(); */	}
	
	string getCurrentSkinPath() { return m_CurrentSkinPath;	}
	void setCurrentSkinPath(string skinpath) { m_CurrentSkinPath = skinpath; Write(); }
	
	int getHorizOverscan() { return m_HorizOverscan; }
	void setHorizOverscan(int value) { m_HorizOverscan = value; Write(); }

	int getVertOverscan() {	return m_VertOverscan; }
	void setVertOverscan(int value)	{ m_VertOverscan = value; Write(); }

	int getEnableLetterbox() { return m_EnableLetterbox; }
	void setEnableLetterbox(int value) { m_EnableLetterbox = value; Write(); }

	int getTempStatus() { return m_TempStatus; }
	void setTempStatus(int value) {	m_TempStatus  = value; Write(); }
	
	int getFanSpeed() {	return m_FanSpeed; }
	void setFanSpeed(int value) { m_FanSpeed = value; Write(); }

	string getSkin() { return m_skin; }
	void setSkin(string value) { m_skin = value; Write(); }
	
	string getXboxIp() { return m_xboxip; }
	void setXboxIp(string value) { m_xboxip = value; Write(); }

	int getPort() {	return m_port; }
	void setPort(int value) { m_port = value; Write(); }

	string getFtpUser() { return m_ftpuser; }
	void setFtpUser(string value) {	m_ftpuser = value; Write();	}

	string getFtpPass()	{ return m_ftppass;	}
	void setFtpPass(string value) {	m_ftppass=value; Write();}

	string getDebugIp()	{ return m_debugip;	}
	void setDebugIp(string value) {	m_debugip = value; Write();	}

	int getDebugPort() { return m_debugport; }
	void setDebugPort(int value) { m_debugport = value; Write(); }

	int getUseBackground() { return m_UseBackgrounds; }
	void setUseBackground(int value) { m_UseBackgrounds = value; Write(); }

	int getUseScreenshots() { return m_UseScreenshots; }
	void setUseScreenshots(int value) { m_UseScreenshots = value; Write(); }

	int getSemiTransparentBackground() { return m_SemiTransparentBackground; }
	void setSemiTransparentBackground(int value) { m_SemiTransparentBackground = value; Write(); }
	
	int getUsePreviews() { return m_UsePreviews;	}
	void setUsePreviews(int value) { m_UsePreviews = value; Write(); }
	
	int getCels() {	return m_Cels; }
	void setCels(int value)	{ m_Cels = value; Write(); }

	int getDiskStatus()	{ return m_DiskStatus; }
	void setDiskStatus(int value) {	m_DiskStatus = value; Write(); }

	int getParentalControl() { return m_ParentalControl; }
	void setParentalControl(int value) { m_ParentalControl = value;	Write(); }
	
	int getShowIP() { return m_ShowIP; }
	void setShowIP(int value) {	m_ShowIP = value; Write(); }

	int getShowStatus() { return m_ShowStatus; }
	void setShowStatus(int value) { m_ShowStatus = value; Write(); }
	
	string getMusicPath() {	return m_MusicPath;	}
	void setMusicPath(string value)	{ m_MusicPath = value; Write();	}
	
	string getVideoPath() {	return m_VideoPath; }
	void setVideoPath(string value) { m_VideoPath = value; Write();	}

	int getDateFormat() { return m_DateFormat; }
	void setDateFormat(int value) {	m_DateFormat = value; Write(); }
	
	int getTimeFormat()	{ return m_TimeFormat; }
	void setTimeFormat(int value) { m_TimeFormat = value; Write(); }
	
	int getLedType() { return m_LedType; }
	void setLedType(int value) { m_LedType = value;	Write(); }
	
	int getNTPonStartup() {	return m_NTPonStartup; }
	void setNTPonStartup(int value)	{ m_NTPonStartup = value; Write(); }
	
	float getXexVer() {	return m_XexVer; }
	void setXexVer(float value)	{ m_XexVer = value;	Write(); }

	int getGameListVisual() { return m_glVisual; }
	void setGameListVisual(int value) { m_glVisual = value; Write(); }

	int getGameListTransition() { return m_glTransition; }
	void setGameListTransition(int value) { m_glTransition = value; Write(); }
	
	int getShowAvatar() { return m_showAvatar; }
	void setShowAvatar(int value) {	m_showAvatar = value; Write(); }

	int getHorizDisplacement() { return m_HorizDisplacement; }
	void setHorizDisplacement(int value) { m_HorizDisplacement = value;	Write(); }

	int getVertDisplacement() {	return m_VertDisplacement; }
	void setVertDisplacement(int value) { m_VertDisplacement = value; Write(); }

	int getUpdateOnBeta() {	return m_UpdateOnBeta; }
	void setUpdateOnBeta(int value) { m_UpdateOnBeta = value; Write(); }

	int getUpdateXexNotice() { return m_UpdateXexNotice; }
	void setUpdateXexNotice(int value) { m_UpdateXexNotice = value; Write(); }

	int getUpdateSkinNotice() {	return m_UpdateSkinNotice; }
	void setUpdateSkinNotice(int value) { m_UpdateSkinNotice = value; Write(); }

	int getUpdatePluginNotice() { return m_UpdatePluginNotice; }
	void setUpdatePluginNotice(int value) {	m_UpdatePluginNotice = value; Write(); }

	int getInstallerState() { return m_InstallerState; }
	void setInstallerState(int value) {	m_InstallerState = value; Write(); }

	string getWeatherLocation() { return m_zipCode; }
	void setWeatherLocation(string value) {	m_zipCode = value; Write();	}

	string getWeatherLocationName() { return m_weatherLocation; }
	void setWeatherLocationName(string value) {	m_weatherLocation = value; Write();	}

	int getWeatherUnits() {	return m_WeatherUnits; }
	void setWeatherUnits(int value)	{ m_WeatherUnits = value; Write(); }

	void setLocale(string value) { m_Locale = value; Write(); }
	string getLocale() { return m_Locale; }

	string getDVDCopy360() { return m_360Path; }
	void setDVDCopy360(string value) { m_360Path = value; Write(); }

	string getDVDCopyOrig()	{ return m_OrigPath; }
	void setDVDCopyOrig(string value) { m_OrigPath = value; Write(); }

	int getHTTPDisableSecurity() { return m_HTTPDisableSecurity; }
	void setHTTPDisableSecurity(int nValue) { m_HTTPDisableSecurity = nValue; Write(); }

	int getKaiContactOnlineNotice() { return m_NotifyContactOnline; }
	void setKaiContactOnlineNotice(int value) { m_NotifyContactOnline = value; Write(); }

	int getKaiContactOfflineNotice() { 
		return m_NotifyContactOffline; 
	}
	void setKaiContactOfflineNotice(int value) 
	{	
		m_NotifyContactOffline = value; 
		Write(); 
	}

	int getKaiInviteToGameNotice() { return m_NotifyInviteToGame; }
	void setKaiInviteToGameNotice(int value) { m_NotifyInviteToGame = value; Write(); }

	int getKaiContactPMNotice() { return m_NotifyContactPM; }
	void setKaiContactPMNotice(int value) { m_NotifyContactPM = value; Write(); }

	int getKaiOpponentPMNotice() { return m_NotifyOpponentPM; }
	void setKaiOpponentPMNotice(int value) { m_NotifyOpponentPM = value; Write(); }

	int getKaiSettingsRemember() { return m_KaiSettingsRemember; }
	void setKaiSettingsRemember(int value) { m_KaiSettingsRemember = value; Write(); }

	int getKaiSettingsAutoLogin() { return m_KaiSettingsAutoLogin; }
	void setKaiSettingsAutoLogin(int value) { m_KaiSettingsAutoLogin = value; Write(); }

	string getKaiSettingsUsername()	{ return m_KaiSettingsUsername; }
	void setKaiSettingsUsername(string value) { m_KaiSettingsUsername = value; Write(); }

	string getKaiSettingsPassword()	{ return m_KaiSettingsPassword; }
	void setKaiSettingsPassword(string value) { m_KaiSettingsPassword = value; Write(); }

	string getKaiSettingsIP()	{ return m_KaiSettingsIP; }
	void setKaiSettingsIP(string value) { m_KaiSettingsIP = value; Write(); }

	int getSvnRevision(void) { return m_SVNRevision; }

	// Settings for TeamFSD.com
	string getFsdUsername() { return m_FsdUsername; }
	void   setFsdUsername(string value) { m_FsdUsername = value; Write(); }
	string getFsdPassword() { return m_FsdPassword; }
	void   setFsdPassword(string value) { m_FsdPassword = value; Write(); }
	int   getFsdUseLogin() { return m_FsdUseLogin; }
	void   setFsdUseLogin(int value) { m_FsdUseLogin = value; Write(); }

	bool   getFtpServerOn() { return m_FtpServerOn; }
	void   setFtpServerOn(bool value) { m_FtpServerOn = value; Write(); }

	bool   getHttpServerOn() { return m_HttpServerOn; }
	void   setHttpServerOn(bool value) { m_HttpServerOn = value; Write(); }

	bool   getSambaClientOn() { return m_SambaClientOn; }
	void   setSambaClientOn(bool value) { m_SambaClientOn = value; Write(); }

	bool   getSambaServerOn() { return m_SambaServerOn; }
	void   setSambaServerOn(bool value) { m_SambaServerOn = value; Write(); }

	string getSambaClientWorkgroup() { return m_SambaClientWorkgroup; }
	void   setSambaClientWorkgroup(const string& value) { m_SambaClientWorkgroup = value; Write(); }

	string getHostname() { return m_hostname; }
	void   setHostname(const string& value) { m_hostname = value; Write(); }

	int	   getShowStatusAtBoot() { return m_ShowStatusAtBoot; }
	void   setShowStatusAtBoot(int value) { m_ShowStatusAtBoot = value; Write(); }

	int		getDownloadIcon() { return m_DownloadIcon; }
	void	setDownloadIcon(int value) { m_DownloadIcon = value; Write(); }

	int		getDownloadBoxart() { return m_DownloadBoxart; }
	void	setDownloadBoxart(int value) { m_DownloadBoxart = value; Write(); }

	int		getDownloadBanner() { return m_DownloadBanner; }
	void	setDownloadBanner(int value) { m_DownloadBanner = value; Write(); }

	int		getDownloadScreenshots() { return m_DownloadScreenshots; }
	void 	setDownloadScreenshots(int value) { m_DownloadScreenshots = value; Write(); }

	int		getDownloadBackground() { return m_DownloadBackground; }
	void 	setDownloadBackground(int value) { m_DownloadBackground = value; Write(); }

	int		getDownloadVideo() { return m_DownloadVideo; }
	void	setDownloadVideo(int value) { m_DownloadVideo = value; Write(); }

	int		getMaxScreenshots() { return m_MaxScreenshots; }
	void	setMaxScreenshots(int value) { m_MaxScreenshots = value; Write(); }

	string  getHttpUsername() { return m_HttpUsername; }
	void	setHttpUsername(string value) { m_HttpUsername = value; Write(); }

	string  getHttpPassword() { return m_HttpPassword; }
	void	setHttpPassword(string value) { m_HttpPassword = value; Write(); }

	int getFMAdvancedMode() { return m_EnableAdvancedMode; }
	void setFMAdvancedMode(int value) { m_EnableAdvancedMode = value; Write(); }

	int getCoverPoll() { return m_CoverPoll; }
	void setCoverPoll(int value) { m_CoverPoll = value; Write(); }

	string getCoverUser() { return m_CoverUsername; }
	void setCoverUser(string value) { m_CoverUsername = value; Write(); }

	string getCoverPass() { return m_CoverPassword; }
	void setCoverPass(string value) { m_CoverPassword = value; Write(); }

	bool getCoverCheck() { return m_CoverCheck; }
	void setCoverCheck( bool value) { m_CoverCheck = value; Write(); }

	int getCoverQuality() { return m_CoverQuality; }
	void setCoverQuality( int value ) { m_CoverQuality = value; Write(); }

//	void getGameListSort(int type, GAMELIST_SORTSTYLE* sort, GAMELIST_SORTDIRECTION* dir);
//	void setGameListSort(int type, GAMELIST_SORTSTYLE sort, int dir);

	void getGameListSort(int type, int* sort, int* dir, int* view);
	void setGameListSort(int type, int sort, int dir, int view);

	void ResetToDefaultSettings();

	const vector<ScanPath>& getScanPaths() {return ScanPaths; }
	void setScanPaths(const vector<ScanPath>& Value ) { ScanPaths = Value; Write(); }

	const ScanPath* getScanPathByScanPathId(ULONGLONG ScanPathId);

//	vector<string> Paths;
	// Other non-static member functions
private:
	
	int m_ResumeAssetDownload;
	int m_DisableAutoScan;
	string m_weatherLocation;
	string m_zipCode;
	int m_WeatherUnits;

	vector<ScanPath> ScanPaths;
	vector<string> rssPaths;
	int m_MaxScreenshots;
	int m_HorizOverscan;
	int m_VertOverscan;
	int m_HorizDisplacement;
	int m_VertDisplacement;
	int m_EnableLetterbox;
	int m_EnableAdvancedMode;
	int m_TempStatus;
	int m_FanSpeed;
	string m_Locale;
	string m_skin;
	string m_DataPath ;
	string m_DataPathX ;
	string m_CurrentSkinPath;
	string m_xboxip;

	int m_CoverPoll;
	string m_CoverPassword;
	string m_CoverUsername;
	bool m_CoverCheck;
	int m_CoverQuality;
	
	int m_port;
	string m_ftpuser;
	string m_ftppass;
	string m_debugip;
	int m_debugport;
	int m_UseBackgrounds ;
	int m_UseScreenshots;
	int m_SemiTransparentBackground;
	int m_Cels;
	int m_UsePreviews ;
	int m_DiskStatus ;
	int m_ParentalControl;
	int m_ShowIP;
	int m_ShowStatus ;
	int m_SVNRevision;
	int m_glVisual;
	int m_glTransition;
	int m_DateFormat;
	int m_TimeFormat;
	int m_LedType;
	int m_NTPonStartup;
	int m_ShowStatusAtBoot;
	float m_XexVer;
	int m_showAvatar;
	int m_HTTPDisableSecurity;

	int m_DownloadIcon;
	int m_DownloadBoxart;
	int m_DownloadBanner;
	int m_DownloadBackground;
	int m_DownloadScreenshots;
	int m_DownloadVideo;

	string m_HttpUsername;
	string m_HttpPassword;

	string m_KaiSettingsUsername;
	string m_KaiSettingsPassword;
	string m_KaiSettingsIP;
	int m_NotifyContactOnline;
	int m_NotifyContactOffline;
	int m_NotifyInviteToGame;
	int m_NotifyContactPM;
	int m_NotifyOpponentPM;
	int m_KaiSettingsAutoLogin;
	int m_KaiSettingsRemember;

	bool m_FtpServerOn;
	bool m_HttpServerOn;
	bool m_SambaClientOn;
	string m_SambaClientWorkgroup;
	bool m_SambaServerOn;
	string m_hostname;

	string m_FsdUsername;
	string m_FsdPassword;
	int   m_FsdUseLogin;

	bool m_PingPatch;

	// AutoUpdater Variables
	int m_UpdateOnBeta;
	int m_UpdateXexNotice;
	int m_UpdateSkinNotice;
	int m_UpdatePluginNotice;
	int m_InstallerState;   // 0- do not run installer, 1 - run installer, 2- error during installer (re run or manual update)

	string m_MusicPath;
	string m_VideoPath;
	string m_360Path;
	string m_OrigPath;

	std::map<int, SortIds*> m_sortIds; 

	void DefaultSettings();
	void Read();
	void Write();

	SETTINGS();
	~SETTINGS() {}
	SETTINGS(const SETTINGS&);                 // Prevent copy-construction
	SETTINGS& operator=(const SETTINGS&);      // Prevent assignment
};
