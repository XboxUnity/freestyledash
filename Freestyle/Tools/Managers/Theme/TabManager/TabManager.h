#pragma once
#include "../../../Generic/xboxtools.h"

enum TabManagerFlags {
	// Flags for Menu Data
	TM_MENU_LOADED,
	TM_MENU_PREVIEW,
	TM_MENU_ACTIVE,
	// Flags for Asset Data
	TM_ASSET_BKG,
	TM_ASSET_ICON,
	TM_ASSET_REFLECT,
	// Flags for Color
	TM_COLOR_RED,
	TM_COLOR_GREEN,
	TM_COLOR_BLUE,
	TM_COLOR_ALPHA
};

typedef struct
{
	int TabIndex;
	int MenuIndex;
	int BackgroundVisual;
	int BackgroundIndex;
	int IconVisual;
	int IconIndex;
	int ReflectionVisual;
	int ReflectionIndex;
	bool AnimationState;
	bool GradientState;
	bool IconState;
	DWORD MainTextColor;
	DWORD ReflectionTextColor;
	string TabDisplayText;
}TMTabElement;

typedef struct
{
	int MenuIndex;
	int TabElementCount;
	DWORD MenuTextColor;
	string MenuDisplayText;
	map<int, TMTabElement> m_TabElement;
} TMMenuElement;

typedef struct
{
	string Name;
	string ImagePath;
	int Index;
	int Visual;
} AssetData;

typedef struct
{
	string VisualName;
	string VisualDescription;
	string PreviewPath;
	int nBkgListSize;
	int nIconListSize;
	int nReflectListSize;
} VisualData;

typedef struct
{
	string VisualName;
	string VisualDescription;
	string PreviewPath;
	map<int, AssetData> m_IconAsset;
	map<int, AssetData> m_BkgAsset;
	map<int, AssetData> m_ReflectAsset;
} VisualList;

typedef struct
{
	string ContentDisplayText;
	int ContentBkgIndex;
	int ContentBkgVisual;
	int ContentIconVisual;
	int ContentIconIndex;
	int ContentReflectVisual;
	int ContentReflectIndex;
	bool ContentAnimState;
	bool ContentGradientState;
	bool ContentIconState;
	DWORD ContentMainTextColor;
	DWORD ContentReflectTextColor;
} TabDataPackage;

class iThemeTabObserver{
public :
	virtual void handleContentTabChange(int nParent, int nChild, int nFlag) = 0;
	virtual void handleMenuRefresh( int nFlag ) = 0;
};

class TabManager
{
public:
	static TabManager& getInstance()
	{
		static TabManager singleton;
		return singleton;
	}
	
	void setFocus( int nParent, int nChild) { curParentFocus = nParent; curChildFocus = nChild; }
	void setFocusParent( int nParent )		{ curParentFocus = nParent; curChildFocus = curChildFocus; }
	void setFocusChild( int nChild )		{ curParentFocus = curParentFocus; curChildFocus = nChild; }

	int getFocusChild( void )				{ return curChildFocus; }
	int getFocusParent( void )				{ return curParentFocus; }
	
	int getParentCount( int nFlag );
	int getChildCount( int nParent, int nFlag );
	int getVisualCount( void );
	int getAssetCount( int nVisual, int nFlag );

	// RunTime Modificatoins - Title
	int setAlternateText( int nParent, int nChild, string szText, int nFlag);
	int setDefaultText( int nParent, int nChild, int nFlag);

	// RunTime Modifications - Slot Image
	int setAlternateSlot( int nParent, int nChild, string szPath, int nFlag );
	int setDefaultSlot( int nParent, int nChild, int nFlag );

	TabDataPackage requestTabDataPackage( int nParent, int nChild, int nFlag );
	void submitTabDataPackage( int nParent, int nChild, TabDataPackage tabData, int nFlag);
	
	void forceMenuRefresh( int nFlag );
	string getTabDisplayText( int nParent, int nChild, int nFlag );
	void copyMenuMap( int srcMenuMap, int destMenuMap, bool bRefreshMenu );
	
	AssetData getAssetData( int nVisual, int nIndex, int nFlag);
	VisualData getVisualData( int nVisual );

	void SaveXML_MenuSettings( int nFlag );

	// Functions to add and remove yourself from Observer List
	void add(iThemeTabObserver& ref);
	void remove(iThemeTabObserver& ref);
	bool isDoneLoading()
	{
		return doneLoading;
	};
private:
	// Focus Variables
	int curParentFocus;
	int curChildFocus;
	bool doneLoading;
	map<int, VisualList> m_VisualList;

	// Storage of Loaded, Preview, and Active TabMaps
	map<int, TMMenuElement> m_LoadedMenuMap;
	map<int, TMMenuElement> m_PreviewMenuMap;
	map<int, TMMenuElement> m_ActiveMenuMap;

	void LoadXML_VisualList( void );
	void LoadXML_MenuSettings( void );
	void LoadXML_VisualData( int nIndex, string filePath );

	// Map of all children registered with the VisualManager
	std::map<iThemeTabObserver* const,iThemeTabObserver* const> _observers;
	typedef std::map<iThemeTabObserver* const, iThemeTabObserver* const> item;
	// Functions that trigger specific events within each child

	void _notifyContentTabChange( int nParent, int nChild, int nFlag );
	void _notifyMenuRefresh( int nFlag );

	TabManager();			                             // Private constructor
	~TabManager() {}
	TabManager(const TabManager&);                 // Prevent copy-construction
	TabManager& operator=(const TabManager&);      // Prevent assignment
};
