#include "stdafx.h"
#include "ScnOptionsMain.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/Managers/Skin/SkinManager.h"


CScnOptionsMain::CScnOptionsMain()
{
	// Initialize our strings here
}

CScnOptionsMain::~CScnOptionsMain()
{
	// Clean up our allocated memory hedre

}

HRESULT CScnOptionsMain::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_SettingList ) {
		ShowSetting(m_SettingList.GetCurSel());
		currentSelection = m_SettingList.GetCurSel();
		m_SettingTitle.SetText(m_SettingList.GetText(currentSelection));

	} else {
		// Somethign else changed
	}

	return S_OK;
}

HRESULT CScnOptionsMain::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	// First, let's initialize all of our xui controls
	InitializeChildren();

	currentSelection = 0;

	//Set Initial Title Text
	m_SettingTitle.SetText(m_SettingList.GetText(currentSelection));

	return S_OK;
}

HRESULT CScnOptionsMain::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if( hObjPressed == m_SettingList ) {

		switch(m_SettingList.GetCurSel())
		{
		case GeneralSettings:
			SkinManager::getInstance().setScene("Settings_General.xur", *this, true, "Settings_General");
			break;
		case ContentSettings:
			SkinManager::getInstance().setScene("Settings_Content.xur", *this, true, "Settings_Content");
			break;
		case SystemSettings:
			SkinManager::getInstance().setScene("Settings_System.xur", *this, true, "Settings_System");
			break;
		case XlinkKai:
			SkinManager::getInstance().setScene("Settings_Kai.xur", *this, true, "Settings_Kai");
			break;
		}

		bHandled = TRUE;

	} else {
		// Button press not recognized
	}

	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsMain::InitializeChildren( void )
{	
	GetChildById(L"SettingList", &m_SettingList);
	GetChildById(L"SettingTitle", &m_SettingTitle);
	GetChildById(L"SettingsGeneral", &m_SettingScenes[0]);
	GetChildById(L"SettingsContent", &m_SettingScenes[1]);
	GetChildById(L"SettingsSystem", &m_SettingScenes[2]);
	GetChildById(L"SettingsKai", &m_SettingScenes[3]);

	return S_OK;
}

void CScnOptionsMain::ShowSetting(int value)
{
	for(int i = 0; i < SettingListSize; i++)
	{
		if( i == value )
			m_SettingScenes[i].SetShow(true);
		else
			m_SettingScenes[i].SetShow(false);
	}
}