#include "stdafx.h"
#include "ScnParentContentSettings.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/Managers/Skin/SkinManager.h"


CScnParentContentSettings::CScnParentContentSettings()
{
	// Initialize our strings here
}

CScnParentContentSettings::~CScnParentContentSettings()
{
	// Clean up our allocated memory hedre
}

HRESULT CScnParentContentSettings::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_SettingList ) 
	{
		ShowSetting(m_SettingList.GetCurSel());
		currentSelection = m_SettingList.GetCurSel();
		m_SettingTitle.SetText(m_SettingList.GetText(currentSelection));
	} 
	else 
	{
		// Somethign else changed
	}

	return S_OK;
}

HRESULT CScnParentContentSettings::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	// First, let's initialize all of our xui controls
	InitializeChildren();

	currentSelection = 0;

	//Set Initial Title Text
	m_SettingTitle.SetText(m_SettingList.GetText(currentSelection));

	return S_OK;
}

HRESULT CScnParentContentSettings::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if( hObjPressed == m_SettingList ) 
	{
		if(currentSelection == 4)//Manage Path Selected
		{
			PathManager->AddPath();
			m_SettingScenes[currentSelection].SetFocus();
			inChild = true; // Child Scn has Focus
		}
		else
		{
			m_SettingScenes[currentSelection].SetFocus(); // Give the item focus
			inChild = true; // Child Scn has Focus
		}
		bHandled = TRUE;
	} 
	else if ( hObjPressed == m_Back ) 
	{
		HandleBack();//Check Manage Paths for back handle

		bHandled = TRUE;
	}

	// Return successfully
	return S_OK;
}

HRESULT CScnParentContentSettings::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
	if(hObjSource == m_SettingFocus)
	{
		m_SettingScenes[currentSelection].SetFocus();
		inChild = true; // Child Scn has Focus
	}
	else if(hObjSource == m_SettingList)
	{
		inChild = false;
	}
	    
    bHandled = TRUE;
    
    return S_OK ;
}

HRESULT CScnParentContentSettings::InitializeChildren( void )
{	
	VOID* pObj;

	GetChildById(L"Back", &m_Back);
	GetChildById(L"SettingList", &m_SettingList);
	GetChildById(L"SettingTitle", &m_SettingTitle);
	GetChildById(L"SettingFocus", &m_SettingFocus);
	GetChildById(L"SettingsGameList", &m_SettingScenes[0]);
	GetChildById(L"SettingsMarketplace", &m_SettingScenes[1]);
	GetChildById(L"SettingsCustomCover", &m_SettingScenes[2]);
	GetChildById(L"SettingsScanSettings", &m_SettingScenes[3]);
	GetChildById(L"SettingsGamePaths", &m_SettingScenes[4]);

	// Gain Access to PathManager Class
	XuiObjectFromHandle(m_SettingScenes[4], &pObj);
	PathManager = (CScnManagePaths*)pObj;

	// Track Focus location
	inChild = false;

	return S_OK;
}

void CScnParentContentSettings::ShowSetting(int value)
{
	for(int i = 0; i < SettingListSize; i++)
	{
		if( i == value )
			m_SettingScenes[i].SetShow(true);
		else
			m_SettingScenes[i].SetShow(false);
	}
}

void CScnParentContentSettings::HandleBack()
{
	//Pass Handle to PathManager
	if (PathManager->HandleBack() != 1)
	{
		if(inChild)
		{
			m_SettingList.SetFocus(XUSER_INDEX_ANY);
			inChild = false;
		}
		else
			NavigateBack(XUSER_INDEX_ANY);
	}
}