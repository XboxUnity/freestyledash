#include "stdafx.h"
#include "ScnOptionsSystemSettings.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SMC/smc.h"


HRESULT CScnOptionsSystemSettings::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_DateFormat ) {
		m_szSelectedDate = m_DateFormat.GetCurSel();

	} else if (hObjSource == m_TimeFormat) {
		m_szSelectedTime = m_TimeFormat.GetCurSel();
	}

	return S_OK;
}



HRESULT CScnOptionsSystemSettings::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled )
{
	// Handle Vertical Overscan Slider First
	if( hObjSource == m_VerticalOverscan ) {
		int nOldValue = SETTINGS::getInstance().getVertOverscan();
		int nNewValue = pNotifyValueChangedData->nValue;

		wstring szOverscanDisplay = sprintfaW(L"%dpx", nNewValue);
		m_VertOverscanText.SetText(szOverscanDisplay.c_str());
		if( nOldValue != nNewValue )
			SETTINGS::getInstance().setVertOverscan(nNewValue);

		bHandled = TRUE;
	// Handle Horizontal Overscan
	} else if( hObjSource == m_HorizontalOverscan ) {
		int nOldValue = SETTINGS::getInstance().getHorizOverscan();
		int nNewValue = pNotifyValueChangedData->nValue;

		wstring szOverscanDisplay = sprintfaW(L"%dpx", nNewValue);
		m_HorizOverscanText.SetText(szOverscanDisplay.c_str());
		if( nOldValue != nNewValue )
			SETTINGS::getInstance().setHorizOverscan(nNewValue);

		bHandled = TRUE;
	// Handle FanSpeed Adjustment
	} else if( hObjSource == m_FanSpeed ) {
		int nOldSpeed = SETTINGS::getInstance().getFanSpeed(); 
		int nNewSpeed =  pNotifyValueChangedData->nValue;

		if (nOldSpeed != nNewSpeed) { 
			SETTINGS::getInstance().setFanSpeed(nNewSpeed);
			smc mySmc; mySmc.SetFanSpeed(1, nNewSpeed);
			wstring szSpeedText = L"";
			nNewSpeed < 45 ? szSpeedText = L"Auto" : szSpeedText = sprintfaW(L"%d%%", nNewSpeed);
			m_FanSpeedText.SetText(szSpeedText.c_str());
		}
		bHandled = TRUE;

	} else {
		// Unrecognized control changed
	}

	return S_OK;
}

HRESULT CScnOptionsSystemSettings::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	// First, let's initialize all of our xui controls
	InitializeChildren();

	// After initialization, let's fill the controls with stored setting values
	ReadUserSettings();

	// Set Timers for Date and Time
	//SetTimer(TM_CLOCK, 1000);

	return S_OK;
}



HRESULT CScnOptionsSystemSettings::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
    switch( pTimer->nId )
    {
	case TM_CLOCK:
		UpdateDateAndTimeLists();
		break;
	case TM_KEYBOARD:
		if(XHasOverlappedIoCompleted(&keyboard))
        {
			KillTimer(TM_KEYBOARD);
			bHandled = TRUE;
			if(keyboard.dwExtendedError == ERROR_SUCCESS)
            {

				string Temp = wstrtostr(buffer);
				if (UserOrPass == 1) {
					SETTINGS::getInstance().setFtpUser(Temp);
				} else if (UserOrPass == 2) {
					SETTINGS::getInstance().setFtpPass(Temp);
				} else if (UserOrPass == 3) {
					SETTINGS::getInstance().setHttpUsername(Temp);
				} else if (UserOrPass == 4) {
					SETTINGS::getInstance().setHttpPassword(Temp);
				}
				UserOrPass = 0;
			}				
		}
		break;
	default:
		break;
	};
			
    // Return Successfully
	bHandled = TRUE;
    return S_OK;
}

HRESULT CScnOptionsSystemSettings::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if( hObjPressed == m_DateFormat ) {
		bHandled = TRUE;

	} else if( hObjPressed == m_TimeFormat ) {
		bHandled = TRUE;

	} else if( hObjPressed == m_SetFormat ) {
		SETTINGS::getInstance().setDateFormat(m_szSelectedDate);
		SETTINGS::getInstance().setTimeFormat(m_szSelectedTime);

		bHandled = TRUE;

	} else if( hObjPressed == m_EnableLetterbox ) {
		SETTINGS::getInstance().setEnableLetterbox(1 - SETTINGS::getInstance().getEnableLetterbox());
		m_EnableLetterbox.SetCheck(SETTINGS::getInstance().getEnableLetterbox());
		bHandled = TRUE;

	} else if (hObjPressed == m_FTPUser ) {
		bHandled = TRUE;
		buffer = new WCHAR[200];
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));
		FTPUser = strtowstr(SETTINGS::getInstance().getFtpUser());
		XShowKeyboardUI(0,VKBD_DEFAULT,FTPUser.c_str(),L"Change FTP User name",L"Enter a new User name",buffer,200,&keyboard);
		UserOrPass = 1;
		SetTimer(TM_KEYBOARD, 50);

	} else if (hObjPressed == m_FTPPass ) {
		bHandled = TRUE;
		buffer = new WCHAR[200];
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));
		FTPPass = strtowstr(SETTINGS::getInstance().getFtpPass());
		XShowKeyboardUI(0,VKBD_DEFAULT,FTPPass.c_str(),L"Change FTP password",L"Enter a new password",buffer,200,&keyboard);
		UserOrPass = 2;
		SetTimer(TM_KEYBOARD, 50);
	} else if (hObjPressed == m_HTTPUser ) {
		bHandled = TRUE;
		buffer = new WCHAR[200];
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));
		HTTPUser = strtowstr(SETTINGS::getInstance().getHttpUsername());
		XShowKeyboardUI(0, VKBD_DEFAULT, HTTPUser.c_str(), L"Change HTTP Username", L"Enter a new Username", buffer, 200, &keyboard);
		UserOrPass = 3;
		SetTimer(TM_KEYBOARD, 50);
	} else if (hObjPressed == m_HTTPPass ) {
		bHandled = TRUE;
		buffer = new WCHAR[200];
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));
		HTTPPass = strtowstr(SETTINGS::getInstance().getHttpPassword());
		XShowKeyboardUI(0, VKBD_DEFAULT, HTTPPass.c_str(), L"Change HTTP Password", L"Enter a new Password", buffer, 200, &keyboard);
		UserOrPass = 4;
		SetTimer(TM_KEYBOARD, 50);
	} else if (hObjPressed == m_FTPEnable ) {
		bool temp = SETTINGS::getInstance().getFtpServerOn();
		SETTINGS::getInstance().setFtpServerOn(!temp);
	} else if (hObjPressed == m_HTTPEnable ) {
		bool temp = SETTINGS::getInstance().getHttpServerOn();
		SETTINGS::getInstance().setHttpServerOn(!temp);
	} else if (hObjPressed == m_HTTPAuth ) {
		bool temp = SETTINGS::getInstance().getHTTPDisableSecurity();
		SETTINGS::getInstance().setHTTPDisableSecurity(!temp);
	}


	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsSystemSettings::UpdateDateAndTimeLists( void )
{
    SYSTEMTIME LocalSysTime;
    GetLocalTime( &LocalSysTime );

	m_DateFormat.SetText(m_DateFormat.GetCurSel(), strtowstr(GetDate(m_DateFormat.GetCurSel(), LocalSysTime)).c_str());

	m_TimeFormat.SetText(m_TimeFormat.GetCurSel(), strtowstr(GetTime(m_TimeFormat.GetCurSel(), LocalSysTime)).c_str());

	return S_OK;
}

HRESULT CScnOptionsSystemSettings::ReadUserSettings( void )
{
	SYSTEMTIME LocalSysTime;
    GetLocalTime( &LocalSysTime );

	// Initialize Date and Time Controls
	for(unsigned int nCount = 0; nCount < 5; nCount++)
		m_DateMap.push_back(GetDate(nCount, LocalSysTime));
	for(unsigned int nCount = 0; nCount < 4; nCount++)
		m_TimeMap.push_back(GetTime(nCount, LocalSysTime));
	
	m_szCurrentDate = SETTINGS::getInstance().getDateFormat();
	m_szCurrentTime = SETTINGS::getInstance().getTimeFormat();

	// Fill List Items
	m_DateFormat.DeleteItems(0, m_DateFormat.GetItemCount());
	m_DateFormat.InsertItems(0, m_DateMap.size());

	for (unsigned int x = 0; x < m_DateMap.size(); x++)
	{
		m_DateFormat.SetText(x, strtowstr(m_DateMap.at(x)).c_str());
	}

	m_DateFormat.SetCurSel(m_szCurrentDate);
	m_DateFormat.SetTopItem(m_szCurrentDate);

	m_TimeFormat.DeleteItems(0, m_TimeFormat.GetItemCount());
	m_TimeFormat.InsertItems(0, m_TimeMap.size());

	for (unsigned int x = 0; x < m_TimeMap.size(); x++)
	{
		m_TimeFormat.SetText(x, strtowstr(m_TimeMap.at(x)).c_str());
	}

	m_TimeFormat.SetCurSel(m_szCurrentTime);
	m_TimeFormat.SetTopItem(m_szCurrentTime);


	//UpdateDateAndTimeLists();

	// Initialize Overscan Controls
	m_VerticalOverscan.SetValue(SETTINGS::getInstance().getVertOverscan());
	m_HorizontalOverscan.SetValue(SETTINGS::getInstance().getHorizOverscan());
	
	int nVertValue = 0; m_VerticalOverscan.GetValue(&nVertValue);
	m_VertOverscanText.SetText(sprintfaW(L"%dpx", nVertValue).c_str());
	int nHorizValue = 0; m_HorizontalOverscan.GetValue(&nHorizValue);
	m_HorizOverscanText.SetText(sprintfaW(L"%dpx", nHorizValue).c_str());

	m_EnableLetterbox.SetCheck(SETTINGS::getInstance().getEnableLetterbox());

	// Initialize Security Controls
	m_FTPEnable.SetCheck(SETTINGS::getInstance().getFtpServerOn());
	m_HTTPEnable.SetCheck(SETTINGS::getInstance().getHttpServerOn());
	m_HTTPAuth.SetCheck(SETTINGS::getInstance().getHTTPDisableSecurity());
	
	// Intialize Fanspeed Controls
	m_FanSpeed.SetValue(SETTINGS::getInstance().getFanSpeed());
	int nFanSpeed = 0; m_FanSpeed.GetValue(&nFanSpeed);
	m_FanSpeedText.SetText(nFanSpeed < 45 ? L"Auto" : sprintfaW(L"%d%%", nFanSpeed).c_str());

	return S_OK;
}

HRESULT CScnOptionsSystemSettings::InitializeChildren( void )
{	

	// Initialize Date/Time Setting Controls
	GetChildById(L"DateFormat", &m_DateFormat);
	GetChildById(L"TimeFormat", &m_TimeFormat);
	GetChildById(L"SetFormat", &m_SetFormat);

	// Initalize Security Setting Controls
	GetChildById(L"FTPSetUser", &m_FTPUser );
	GetChildById(L"FTPSetPass", &m_FTPPass );
	GetChildById(L"FTPServer", &m_FTPEnable );
	GetChildById(L"HttpServer", &m_HTTPEnable );
	GetChildById(L"HttpAuth", &m_HTTPAuth);
	GetChildById(L"HTTPSetUser", &m_HTTPUser);
	GetChildById(L"HTTPSetPass", &m_HTTPPass);

	// Initialize Overscan Controls
	GetChildById(L"VertOverscan", &m_VerticalOverscan);
	GetChildById(L"HorizOverscan", &m_HorizontalOverscan);
	GetChildById(L"VerticalOverscanText", &m_VertOverscanText);
	GetChildById(L"HorizontalOverscanText", &m_HorizOverscanText);
	GetChildById(L"LetterBox", &m_EnableLetterbox);

	// Fan Speed Controls
	GetChildById(L"FanSpeed", &m_FanSpeed);
	GetChildById(L"FanSpeedText", &m_FanSpeedText);

	return S_OK;
}