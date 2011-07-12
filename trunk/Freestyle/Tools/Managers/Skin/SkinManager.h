#pragma once
#include "../../../Application/FreeStyleUIApp.h"
#include "XML/SkinXml.h"


class skinitem {
	public:
		string SkinFileName;
		string SkinPath;
		string PreviewPath;
		string IconPath;
		string FullSkinPath;
		string XZPPath;
		string xmlpath;
		string FavoriteIconPath;
		string Author;
		string Version;
		bool isCompressed;
		string SkinName;
		bool DisplayFPS;
		bool DisplayProjectTitle;
		bool DisplayCPURender;
		bool DisplayFreeMEM;
		bool HasDefaultFont;
		int MinVer;
		int MaxVer;
		SceneInfo  m_SceneInfo;
		vector<FontInfo> Fonts;
};


class SkinManager
{
public:

	static SkinManager& getInstance()
	{
		static SkinManager singleton;
		return singleton;
	}
  
	string getCurrentSkin();

	map<string, HXUIOBJ> getXurObjectHandle;

	map<string, skinitem> getAvailableSkins();

	void setSkin( CFreestyleUIApp& g_uiApp,string firstScene, bool firstLoad = false);
	
	void setScene(string sceneName, HXUIOBJ curScene, bool stayVisible, string objectRef = "", void * pvInitData = 0);

	string getCurrentScenePath();

	bool getDisplayFPS();

	bool getDisplayCPURender();
	
	bool getDisplayProjectTitle();

	bool getDisplayFreeMem();
	
	string getFavoriteIconPath();	

	void RegisterFonts(CFreestyleUIApp& g_uiApp, string path);

//	void ReadSkinXML(skinitem* skin);

//	void WriteSkinXML(string Setting, string Value);

	void WriteToXML(string RelativePath, string Setting, string Value);
	
	bool isCurrentCompressed();

	string getCurrentxmlPath();

	string getCurrentSkinFileName();

	string getCurrentFullSkinPath();

	string getCurrentSkinPath();

	string getCurrentXZPSkinPath();

	void SetCurrentObj( HXUIOBJ hObj )
	{
		m_hObj = hObj;
	}

	HXUIOBJ GetCurrentObj( void )
	{
		return m_hObj;
	}

	HRESULT getSettingMap(map<string, string>* temp, string sceneName, string instanceId);


private:
	
	skinitem CurrentSkin;
	SettingMap  m_SettingMap;
	InstanceInfo  m_InstanceInfo;

	int m_XexVer;
	string m_BaseSkinPath;
	map<string , skinitem> m_skinList;
	HXUIOBJ m_hObj;

	skinitem Skin;


	void DefaultSettings();

	void setActiveSkin();

	void getSkinList();

	void loadSkin( CFreestyleUIApp& g_uiApp , string firstScene, bool firstLoad = false);

	void loadScene(string sceneName, HXUIOBJ curScene, bool stayVisible, string objectRef = "", void * pvInitData = 0);

	void ReadSkinSettings(skinitem* Skin);

//	void Write(string Setting, string Value);

	void ExtractSkinInfo(skinitem* skin);
	
	void ExtractTempSkinInfo();


	SkinManager(){DefaultSettings();}
	~SkinManager(){}
	SkinManager(const SkinManager&);                 // Prevent copy-construction
	SkinManager& operator=(const SkinManager&);      // Prevent assignment

};
