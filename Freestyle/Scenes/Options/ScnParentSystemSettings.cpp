#include "stdafx.h"
#include "ScnParentSystemSettings.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/Managers/Skin/SkinManager.h"


CScnParentSystemSettings::CScnParentSystemSettings()
{
	// Initialize our strings here
}

CScnParentSystemSettings::~CScnParentSystemSettings()
{
	// Clean up our allocated memory hedre
}

HRESULT CScnParentSystemSettings::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_SettingList ) {

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

HRESULT CScnParentSystemSettings::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	// First, let's initialize all of our xui controls
	InitializeChildren();

	currentSelection = 0;

	//Set Initial Title Text
	m_SettingTitle.SetText(m_SettingList.GetText(currentSelection));

	return S_OK;
}

HRESULT CScnParentSystemSettings::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if( hObjPressed == m_SettingList ) 
	{
		m_SettingScenes[currentSelection].SetFocus();
		inChild = true; // Child Scn has Focus
		bHandled = TRUE;
	} 
	else if ( hObjPressed == m_Back ) 
	{
		if(inChild)
		{
			m_SettingList.SetFocus(XUSER_INDEX_ANY);
			inChild = false;
		}
		else
			NavigateBack(XUSER_INDEX_ANY);

		bHandled = TRUE;
	}
	else 
	{
		// Button press not recognized
	}

	// Return successfully
	return S_OK;
}

HRESULT CScnParentSystemSettings::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
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

HRESULT CScnParentSystemSettings::InitializeChildren( void )
{	
	GetChildById(L"Back", &m_Back);
	GetChildById(L"SettingList", &m_SettingList);
	GetChildById(L"SettingTitle", &m_SettingTitle);
	GetChildById(L"SettingFocus", &m_SettingFocus);
	GetChildById(L"SettingsTimeDate", &m_SettingScenes[0]);
	GetChildById(L"SettingsSecurity", &m_SettingScenes[1]);
	GetChildById(L"SettingsOverscan", &m_SettingScenes[2]);
	GetChildById(L"SettingsFan", &m_SettingScenes[3]);

	return S_OK;
}

void CScnParentSystemSettings::ShowSetting(int value)
{

	for(int i = 0; i < SettingListSize; i++)
	{
		if( i == value )
			m_SettingScenes[i].SetShow(true);
		else
			m_SettingScenes[i].SetShow(false);
	}
}