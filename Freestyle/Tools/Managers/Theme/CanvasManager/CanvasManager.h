#pragma once
#include "../../../Generic/xboxtools.h"

// DebugMessage Control
#define SHOW_DEBUG	0      // 0 = DebugOff   1 = DebugOn

#define MAX_ANIM_ITEMS 4

typedef enum VMOptionsEnum {
	VM_OPTION_ACTIVE,
	VM_OPTION_LOADED,
	VM_OPTION_INDEX
} VMOptions;

typedef struct
{
	string strBackgroundName;
	string strBackgroundPath;
	string strPreviewPath;
	int nIndex;
	bool bCurrent;
} BackgroundElement;

typedef struct
{
	string strPrefixName;
	string strPrefixID;
	string strPreviewPath;
	int nIndex;
	bool bCurrent;
} PrefixElement;

typedef struct
{
	bool animItem[MAX_ANIM_ITEMS];
} AnimationElement;

class iThemeCanvasObserver{
public :
	virtual void handleBackgroundAChange() = 0;
	virtual void handleBackgroundBChange() = 0;
	virtual void handleAnimationChange() = 0;
	virtual void handlePrefixChange() = 0;
};

class CanvasManager
{
public:
	static CanvasManager& getInstance()
	{
		static CanvasManager singleton;
		return singleton;
	}
	// Functions to add and remove yourself from Observer List
	void add(iThemeCanvasObserver& ref);
	void remove(iThemeCanvasObserver& ref);

	// Functions to save and load settings
	void SaveSettingsToFile();
	void LoadSettingsFromFile();

	void LoadBkgAFromXML();
	void LoadBkgBFromXML();

	// Public Accessor Functions - get
	BackgroundElement getBackgroundA( int nIndex, VMOptions nFlag );
	BackgroundElement getBackgroundB( int nIndex, VMOptions nFlag );
	PrefixElement getPrefix( int nIndex, VMOptions nFlag );
	AnimationElement getAnimation( VMOptions nFlag );

	// Public Accessor Functions - set
	void setBackgroundA(BackgroundElement bkgElement, VMOptions nFlag);
	void setBackgroundA(int nElementIndex, VMOptions nFlag);
	void setBackgroundB(BackgroundElement bkgElement, VMOptions nFlag);
	void setBackgroundB(int nElementIndex, VMOptions nFlag);
	void setPrefix(PrefixElement prefixElement, VMOptions nFlag);
	void setPrefix(int nElementIndex, VMOptions nFlag);
	void setAnimation(AnimationElement animElement, VMOptions nFlag);

	// Public Accessors Functions for Internal Background Changing
	void setTemporaryBkgA(int nIndex);
	void setTemporaryBkgA(BackgroundElement bkgElement);
	void setTemporaryBkgA(string path);
	void setTemporaryBkgB(int nIndex);
	void setTemporaryBkgB(BackgroundElement bkgElement);
	void setTemporaryBkgB(string path);
	void revertBkgA();
	void revertBkgB();

	// Public Accessor Functions - getSize
	int getBackgroundACount( void )		{ return m_BackgroundAList.size(); }
	int getBackgroundBCount( void )		{ return m_BackgroundBList.size(); }
	int getPrefixCount( void )			{ return m_PrefixList.size(); }

private:
	// Maps to store the data loaded from XML
	map<int, BackgroundElement> m_BackgroundAList;
	map<int, BackgroundElement> m_BackgroundBList;
	map<int, PrefixElement> m_PrefixList;

	// Global Storage of Elements
	BackgroundElement m_activeBkgA, m_loadedBkgA;
	BackgroundElement m_activeBkgB, m_loadedBkgB;
	AnimationElement m_activeAnim, m_loadedAnim;
	PrefixElement m_activePrefix, m_loadedPrefix;

	// Map of all children registered with the VisualManager
	std::map<iThemeCanvasObserver* const,iThemeCanvasObserver* const> _observers;
	typedef std::map<iThemeCanvasObserver* const, iThemeCanvasObserver* const> item;
	
	// Functions that trigger specific events within each child
	void _notifyBackgroundAChange();
	void _notifyBackgroundBChange();
	void _notifyPrefixChange();
	void _notifyAnimationChange();

	int nDebugFlag;

	CanvasManager();			                             // Private constructor
	~CanvasManager() {}
	CanvasManager(const CanvasManager&);                 // Prevent copy-construction
	CanvasManager& operator=(const CanvasManager&);      // Prevent assignment
};
