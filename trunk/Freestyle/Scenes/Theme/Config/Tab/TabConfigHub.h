#pragma once
#include "../../../../Tools/Generic/tools.h"
#include "../../../../Tools/Managers/Theme/TabManager/TabManager.h"

class iTabConfigObserver {
public:
	virtual void handleTabPreviewChange(TabDataPackage tabData) = 0;
};

class TabConfigHub
{
public:
	static TabConfigHub& getInstance()
	{
		static TabConfigHub singleton;
		return singleton;
	}

	void setPreviewImage(TabDataPackage tabData);
	
	void setPreviewData( TabDataPackage tabData );
	TabDataPackage getPreviewData( void );

	void add(iTabConfigObserver& ref);
	void remove(iTabConfigObserver& ref);

private:

	TabDataPackage tabConfigData;
	
	std::map<iTabConfigObserver* const,iTabConfigObserver* const> _observers;
	typedef std::map<iTabConfigObserver* const, iTabConfigObserver* const> item;

	void _notifyTabPreviewChange(TabDataPackage tabData);

	TabConfigHub(){ TabManager::getInstance().copyMenuMap(TM_MENU_ACTIVE, TM_MENU_PREVIEW, true); }
	~TabConfigHub(){}
	
	TabConfigHub(const TabConfigHub&);		            // Prevent copy-construction
	TabConfigHub& operator=(const TabConfigHub&);		    // Prevent assignment

};