#include "stdafx.h"

#include "../../../../Tools/Settings/Settings.h"
#include "../../../../Tools/Managers/Theme/TabManager/TabManager.h"

#include "TabConfigHub.h"

// TabConfigHub Class


void TabConfigHub::add(iTabConfigObserver& ref)
{
	_observers.insert(item::value_type(&ref,&ref));

}
void TabConfigHub::remove(iTabConfigObserver& ref)
{
	_observers.erase(&ref);

}

TabDataPackage TabConfigHub::getPreviewData( void )
{
	return tabConfigData;
}
void TabConfigHub::setPreviewData( TabDataPackage tabData )
{
	tabConfigData = tabData;
}

void TabConfigHub::_notifyTabPreviewChange(TabDataPackage tabData)
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleTabPreviewChange(tabData);
    }
}

void TabConfigHub::setPreviewImage(TabDataPackage tabData)
{
	int nFocusParent = TabManager::getInstance().getFocusParent();
	int nFocusChild = TabManager::getInstance().getFocusChild();

	TabDataPackage temp = TabManager::getInstance().requestTabDataPackage(nFocusParent, nFocusChild, TM_MENU_PREVIEW); 

	int FocusIconIndex = temp.ContentIconIndex;
	int FocusBkgIndex = temp.ContentBkgIndex;
	int FocusReflectIndex = temp.ContentReflectIndex;
	string FocusDisplayText = temp.ContentDisplayText;

	tabData.ContentBkgIndex = FocusBkgIndex;
	tabData.ContentIconIndex = FocusIconIndex;
	tabData.ContentReflectIndex = FocusReflectIndex;
	tabData.ContentDisplayText = FocusDisplayText;

//	TabManager::getInstance().submitTabDataPackage(nFocusParent, nFocusChild, tabData, TM_MENU_PREVIEW);
	tabConfigData = tabData;

	_notifyTabPreviewChange(tabData);
	
}