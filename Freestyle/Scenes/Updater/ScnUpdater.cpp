#include "stdafx.h"

#include "ScnUpdater.h"
#include "../../Tools/HTTP/HttpItem.h"
#include "../../Tools/Generic/tools.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/HTTP/HTTPDownloader.h"
#include "../../Tools/XML/XMLReader.h"
#include "../../Application/FreestyleApp.h"

using namespace std;

void CScnUpdater::handleStatusUpdate( string szUpdate )
{
	// Process updates from AutoUpdate class
	if(hasUpdateStatus)
		m_UpdateStatus.SetText(strtowstr(szUpdate).c_str());
}

void CScnUpdater::handleDownloadFinished( int nItem )
{

}

void CScnUpdater::handleLoadingComplete( void )
{
	DebugMsg("CScnUpdater", "List Loading Complete");
	// Fill in the lists
	FillList();
}

CScnUpdater::CScnUpdater()
{
	bChangelogMode = false;
}

CScnUpdater::~CScnUpdater()
{
	AutoUpdater::getInstance().remove(*this);
}

HRESULT CScnUpdater::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnUpdater", "ScnUpdater Initialized");
	InitializeChildren();
	if(CFreestyleApp::getInstance().hasInternetConnection() == false) {
		XNotifyQueueUICustom(L"No network connection detected.");
	} else {
		AutoUpdater::getInstance().add(*this);
		AutoUpdater::getInstance().CheckForUpdates(false);
	}

	//Hide buttons for setting page
	HideButtons();

    return S_OK;
}

HRESULT CScnUpdater::OnSetFocus(HXUIOBJ hObjLosingFocus, BOOL& bHandled)
{
	if(XuiElementTreeHasFocus(this->m_hObj) == TRUE)
		ShowButtons();

	return S_OK;
}


HRESULT CScnUpdater::OnNotifyKillFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
	HXUIOBJ parentLoosing, listParent, checkBoxListParent;
	XuiElementGetParent(pNotifyFocusData->hObjOther , &parentLoosing);
	XuiElementGetParent(parentLoosing, &listParent);
	XuiElementGetParent(listParent, &checkBoxListParent);

	if(parentLoosing == this->m_hObj)//Single parent items (buttons, sliders, parent->item)
	{
		ShowButtons();
	}
	else if(listParent == this->m_hObj)//Two deep (parent->List->control_listItem)
	{
		ShowButtons();
	}
	else if(checkBoxListParent == this->m_hObj)//Three deep (parent->List->ListItemGroup->controlListItem)
	{
		ShowButtons();
	}
	else
	{
		HideButtons();
	}

	return S_OK;
}

HRESULT CScnUpdater::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if(bChangelogMode == true) {
		//Logged into Kai enable the Menu Items
		int FrameStart, FrameEnd;
		this->FindNamedFrame(strtowstr("HideChangelog").c_str(), &FrameStart);
		this->FindNamedFrame(strtowstr("EndHideChangelog").c_str(), &FrameEnd);
		if(FrameStart != -1 && FrameEnd != -1)
		{
			this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
		}
		m_DashUpdates.SetEnable(TRUE);
		m_SkinUpdates.SetEnable(TRUE);
		m_PluginUpdates.SetEnable(TRUE);
		m_ApplyUpdates.SetEnable(TRUE);
		m_PluginUpdates.SetFocus();
		m_SkinUpdates.SetFocus();
		m_DashUpdates.SetFocus();
		bChangelogMode = false;
		bHandled = true;
		return S_OK;
	}

	if(hObjPressed == m_ApplyUpdates)
	{
		ListGroup dashQueue, skinQueue, pluginQueue;
		CreateDownloadQueue(&dashQueue, &skinQueue, &pluginQueue);

		int nSizeA = dashQueue.nListItemCount;
		int nSizeB = skinQueue.nListItemCount;
		int nSizeC = pluginQueue.nListItemCount;

		DebugMsg("CScnUpdater", "Dash Selections:  %d", nSizeA);
		DebugMsg("CScnUpdater", "Skin Selections:  %d", nSizeB);
		DebugMsg("CScnUpdater", "Plugins Selections:  %d", nSizeC);

		if(nSizeA > 0 || nSizeB > 0 || nSizeC > 0)
			AutoUpdater::getInstance().DownloadQueuedItems(dashQueue, skinQueue, pluginQueue);

		bHandled = true;
	}

	if(hObjPressed == m_ShowChangelog)
	{
		//Logged into Kai enable the Menu Items
		int FrameStart, FrameEnd;
		this->FindNamedFrame(strtowstr("ShowChangelog").c_str(), &FrameStart);
		this->FindNamedFrame(strtowstr("EndShowChangelog").c_str(), &FrameEnd);
		if(FrameStart != -1 && FrameEnd != -1)
		{
			this->PlayTimeline(FrameStart, FrameStart, FrameEnd, false, false);
		}
		m_DashUpdates.SetEnable(FALSE);
		m_SkinUpdates.SetEnable(FALSE);
		m_PluginUpdates.SetEnable(FALSE);
		m_ApplyUpdates.SetEnable(FALSE);
		m_ChangeLogEdit.SetFocus();
		bChangelogMode = true;
		bHandled = true;
	}

	if(hObjPressed == m_DashUpdates)
	{
		int nCurrent = m_DashUpdates.GetCurSel();
		int nCount = AutoUpdater::getInstance().getListMap(FILETYPE_DASH).nListItemCount;

		if(m_DashUpdates.GetCheckedItemCount() > 1)
		{
			for( int i = 0; i < nCount; i++)
				m_DashUpdates.SetItemCheck(i, FALSE);

			m_DashUpdates.SetItemCheck(nCurrent, TRUE);
		}

		bHandled = true;
	}

	if(hObjPressed == m_SkinUpdates)
	{
		int nCurrent = m_SkinUpdates.GetCurSel();
		int nCount = AutoUpdater::getInstance().getListMap(FILETYPE_SKIN).nListItemCount;
	
		if(m_SkinUpdates.GetCheckedItemCount() > 1)
		{
			for( int i = 0; i < nCount; i++)
				m_SkinUpdates.SetItemCheck(i, FALSE);

			m_SkinUpdates.SetItemCheck(nCurrent, TRUE);
		}

		bHandled = true;
	}

	if(hObjPressed == m_PluginUpdates)
	{
		int nCurrent = m_PluginUpdates.GetCurSel();
		int nCount = AutoUpdater::getInstance().getListMap(FILETYPE_PLUGIN).nListItemCount;

		if(m_PluginUpdates.GetCheckedItemCount() > 1)
		{

			for( int i = 0; i < nCount; i++)
				m_PluginUpdates.SetItemCheck(i, FALSE);
	
			m_PluginUpdates.SetItemCheck(nCurrent, TRUE);
		}
		bHandled = true;
	}
	return S_OK;   
}

HRESULT CScnUpdater::InitializeChildren( )
{
	HRESULT hr = NULL;
	hr = GetChildById(L"SelectUpdates", &m_SelectUpdates);

	hr = GetChildById(L"DashUpdates", &m_DashUpdates);
	hasDashUpdates = hr == S_OK;

	hr = GetChildById(L"SkinUpdates", &m_SkinUpdates);
	hasSkinUpdates = hr == S_OK;

	hr = GetChildById(L"PluginUpdates", &m_PluginUpdates);
	hasPluginUpdates = hr == S_OK;

	hr = GetChildById(L"UpdateStatus", &m_UpdateStatus);
	hasUpdateStatus = hr == S_OK;

	hr = GetChildById(L"ApplyUpdates", &m_ApplyUpdates);
	hasApplyUpdates = hr == S_OK;

	hr = GetChildById(L"Changelog", &m_ChangeLogEdit);
	hasChangeLogEdit = hr == S_OK;

	hr = GetChildById(L"DashStatus", &m_DashStatus);
	hasDashStatus = hr == S_OK;
	if(hasDashStatus)
		m_DashStatus.SetShow(FALSE);

	hr = GetChildById(L"SkinStatus", &m_SkinStatus);
	hasSkinStatus = hr == S_OK;
	if(hasSkinStatus)
		m_SkinStatus.SetShow(FALSE);

	hr = GetChildById(L"PluginStatus", &m_PluginStatus);
	hasPluginStatus = hr == S_OK;
	if(hasPluginStatus)
		m_PluginStatus.SetShow(FALSE);

	hr = GetChildById(L"ShowChangelog", &m_ShowChangelog);
	hasShowChangelog = hr == S_OK;

	return S_OK;
}

void CScnUpdater::CreateDownloadQueue( ListGroup * dashQueue, ListGroup * skinQueue, ListGroup * pluginQueue)
{
	ListGroup dashList, skinList, pluginList;
	ListGroup retGroup;
	ListItem retList;

	dashList.nListItemCount = 0;
	skinList.nListItemCount = 0;
	pluginList.nListItemCount = 0;

	// Determine which boxes are checked
	int nDashCount = m_DashUpdates.GetItemCount();

	// If the list box has no items in it, there is nothing to download
	if(nDashCount > 0)
	{
		// Get List Array
		retGroup = AutoUpdater::getInstance().getListMap(FILETYPE_DASH);
		
		// Loop through each item in the list and add checked items to dashList listgroup
		for(int i = 0; i < nDashCount; i++)
		{
			if(m_DashUpdates.GetItemCheck(i) == TRUE)
			{
				retList.nFileType = FILETYPE_DASH;
				retList.nListIndex = 0;
				retList.nMapIndex = retGroup.listItems.at(i).nMapIndex;
				retList.szDisplayTitles = retGroup.listItems.at(i).szDisplayTitles;

				dashList.listItems.push_back(retList);
			}
		}
		// Update the listgroup wtih the proper size
		dashList.nListItemCount = dashList.listItems.size();
	}


	// Determine which boxes are checked
	int nSkinCount = m_SkinUpdates.GetItemCount();

	// If the list box has no items in it, there is nothing to download
	if(nSkinCount > 0)
	{
		// Get List Array
		retGroup = AutoUpdater::getInstance().getListMap(FILETYPE_SKIN);
		
		// Loop through each item in the list and add checked items to skinList listgroup
		for(int i = 0; i < nSkinCount; i++)
		{
			if(m_SkinUpdates.GetItemCheck(i) == TRUE)
			{
				retList.nFileType = FILETYPE_SKIN;
				retList.nListIndex = 0;
				retList.nMapIndex = retGroup.listItems.at(i).nMapIndex;
				retList.szDisplayTitles = retGroup.listItems.at(i).szDisplayTitles;

				skinList.listItems.push_back(retList);
			}
		}
		// Update the listgroup wtih the proper size
		skinList.nListItemCount = skinList.listItems.size();
	}

	// Determine which boxes are checked
	int nPluginCount = m_PluginUpdates.GetItemCount();

	// If the list box has no items in it, there is nothing to download
	if(nPluginCount > 0)
	{
		// Get List Array
		retGroup = AutoUpdater::getInstance().getListMap(FILETYPE_PLUGIN);
		
		// Loop through each item in the list and add checked items to pluginList listgroup
		for(int i = 0; i < nPluginCount; i++)
		{
			if(m_PluginUpdates.GetItemCheck(i) == TRUE)
			{
				retList.nFileType = FILETYPE_PLUGIN;
				retList.nListIndex = 0;
				retList.nMapIndex = retGroup.listItems.at(i).nMapIndex;
				retList.szDisplayTitles = retGroup.listItems.at(i).szDisplayTitles;

				pluginList.listItems.push_back(retList);
			}
		}
		// Update the listgroup wtih the proper size
		pluginList.nListItemCount = pluginList.listItems.size();
	}

	// Return Values back to previous function
	*dashQueue = dashList;
	*skinQueue = skinList;
	*pluginQueue = pluginList;
}

void CScnUpdater::FillList()
{
	// Load Changelog
	m_ChangeLogEdit.SetText(strtowstr(AutoUpdater::getInstance().getChangelogDescriptor()->szLogData).c_str());

	ListGroup dashList = AutoUpdater::getInstance().getListMap(FILETYPE_DASH);
	ListGroup skinList = AutoUpdater::getInstance().getListMap(FILETYPE_SKIN);
	ListGroup pluginList = AutoUpdater::getInstance().getListMap(FILETYPE_PLUGIN);

	// Fill in list containing dash updates
	int nDashSize = dashList.nListItemCount;
	if(hasDashUpdates)
	{
		// Clear the list
		int nbCount = m_DashUpdates.GetItemCount();
		m_DashUpdates.DeleteItems(0, nbCount);

		// Insert items
		m_DashUpdates.InsertItems(0, nDashSize);

		// Set the text for each item
		for(int i = 0; i < nDashSize; i++)
		{
			string szDisplayText = dashList.listItems.at(i).szDisplayTitles;
			m_DashUpdates.SetText(i, strtowstr(szDisplayText).c_str());
		}

		if(hasDashStatus){
			if(nDashSize == 0)
				m_DashStatus.SetShow(TRUE);
		}
	}

	// Fill in list containing skin updates
	int nSkinSize = skinList.nListItemCount;
	if(hasSkinUpdates)
	{
		// Clear the list
		int nbCount = m_SkinUpdates.GetItemCount();
		m_SkinUpdates.DeleteItems(0, nbCount);

		// Insert items
		m_SkinUpdates.InsertItems(0, nSkinSize);

		// Set the text for each item
		for(int i = 0; i < nSkinSize; i++)
		{
			string szDisplayText = skinList.listItems.at(i).szDisplayTitles;
			m_SkinUpdates.SetText(i, strtowstr(szDisplayText).c_str());
		}

		if(hasSkinStatus){
			if(nSkinSize == 0)
				m_SkinStatus.SetShow(TRUE);
		}
	}

	// Fill in list containing skin updates
	int nPluginSize = pluginList.nListItemCount;
	if(hasPluginUpdates)
	{
		// Clear the list
		int nbCount = m_PluginUpdates.GetItemCount();
		m_PluginUpdates.DeleteItems(0, nbCount);

		// Insert items
		m_PluginUpdates.InsertItems(0, nPluginSize);

		// Set the text for each item
		for(int i = 0; i < nPluginSize; i++)
		{
			string szDisplayText = pluginList.listItems.at(i).szDisplayTitles;
			m_PluginUpdates.SetText(i, strtowstr(szDisplayText).c_str());
		}

		if(hasPluginStatus){
			if(nPluginSize == 0)
				m_PluginStatus.SetShow(TRUE);
		}
	}
}


void CScnUpdater::ShowButtons()
{
	m_SelectUpdates.SetShow(true);
	m_ApplyUpdates.SetShow(true);
	m_ShowChangelog.SetShow(true);
}

void CScnUpdater::HideButtons()
{
	m_SelectUpdates.SetShow(false);
	m_ApplyUpdates.SetShow(false);
	m_ShowChangelog.SetShow(false);
}