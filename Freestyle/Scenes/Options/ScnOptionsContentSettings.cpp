#include "stdafx.h"
#include "ScnOptionsContentSettings.h"

#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/ContentList/ContentItemNew.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/ContentList/ContentManager.h"


CScnOptionsContentSettings::CScnOptionsContentSettings()
{
	// Initialize our strings here
	m_szKeyboardUsernameCaption = L"Covers.JQE360.com Username";
	m_szKeyboardUsernamePrompt = L"Enter your Covers.JQE360.com username...";
	m_szKeyboardPasswordCaption = L"Covers.JQE360.com Password";
	m_szKeyboardPasswordPrompt = L"Enter your Covers.JQE360.com password...";
}

CScnOptionsContentSettings::~CScnOptionsContentSettings()
{
	// Clean up our allocated memory hedre

}

HRESULT CScnOptionsContentSettings::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId) 
	{
	case TM_KEYBOARD:
		if(XHasOverlappedIoCompleted(&m_kbOverlapped)) {
			bHandled = TRUE;
			KillTimer(TM_KEYBOARD);

			if(m_kbOverlapped.dwExtendedError == ERROR_SUCCESS) {
				if(m_dwKeyboardMode == KBMODE_USERNAME) {
					m_dwKeyboardMode = KBMODE_NONE;
					m_CoverUser.SetText(m_kbBuffer);
					string m_szUsername = wstrtostr(m_kbBuffer);
					SETTINGS::getInstance().setCoverUser(m_szUsername);

				} else if(m_dwKeyboardMode == KBMODE_PASSWORD) {
					m_dwKeyboardMode = KBMODE_NONE;
					string m_szPassword = wstrtostr(m_kbBuffer);
					m_CoverPass.SetText(strtowstr(GetPasswordMask(m_szPassword.length(), "*")).c_str());					
					SETTINGS::getInstance().setCoverPass(m_szPassword);

				} else {
					// Unhandled Keyboard Mode
				}
			}
		}
		break;
	default:
		break;
	};

	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsContentSettings::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_LocaleList ) {
		m_szSelectedLocale = wstrtostr(m_LocaleList.GetText(m_LocaleList.GetCurSel()));

	} else {
		// Somethign else changed

	}

	return S_OK;
}

HRESULT CScnOptionsContentSettings::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	// Create Locale Map
	GetLocaleMap(m_LocaleMap);
	
	// First, let's initialize all of our xui controls
	InitializeChildren();

	// After initialization, let's fill the controls with stored setting values
	ReadUserSettings();

	// Setup our Buffers and Variables for Keyboard UI
	m_kbBuffer = new WCHAR[512];
	m_dwKeyboardMode = KBMODE_NONE;


	return S_OK;
}

HRESULT CScnOptionsContentSettings::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if( hObjPressed == m_UseBackgrounds ) {
		SETTINGS::getInstance().setUseBackground(1 - SETTINGS::getInstance().getUseBackground());
		m_UseBackgrounds.SetCheck(SETTINGS::getInstance().getUseBackground());
		bHandled = TRUE;
	} else if( hObjPressed == m_UseScreenshots ) {
		SETTINGS::getInstance().setUseScreenshots(1 - SETTINGS::getInstance().getUseScreenshots());
		m_UseScreenshots.SetCheck(SETTINGS::getInstance().getUseScreenshots());
		bHandled = TRUE;
	} else if( hObjPressed == m_GameListVisual ) {
		SETTINGS::getInstance().setGameListVisual(m_GameListVisual.GetCurSel());
		for (unsigned int nCount = 0; nCount < 3; nCount++) {
			m_GameListVisual.SetItemCheck(nCount, FALSE);
		}
		m_GameListVisual.SetItemCheck(SETTINGS::getInstance().getGameListVisual(), TRUE);
		bHandled = TRUE;
	} else if( hObjPressed == m_GameListTransition ) {
		SETTINGS::getInstance().setGameListTransition(m_GameListTransition.GetCurSel());
		for (unsigned int nCount = 0; nCount < 3; nCount++) {
			m_GameListTransition.SetItemCheck(nCount, FALSE);
		}
		m_GameListTransition.SetItemCheck(SETTINGS::getInstance().getGameListTransition(), TRUE);
		bHandled = TRUE;
	
	} else if( hObjPressed == m_LocaleList ) {
		// Nothing happens here

	} else if( hObjPressed == m_SetLocale ) {
		if( strcmp(m_szSelectedLocale.c_str(), "Select New Locale") != 0 ) {
			string szLocaleCode = m_LocaleMap[m_szSelectedLocale];
			SETTINGS::getInstance().setLocale(szLocaleCode);
			m_szCurrentLocale = m_szSelectedLocale;
			m_CurrentLocale.SetText(strtowstr(m_szCurrentLocale).c_str());

		} else {
			XNotifyQueueUICustom(L"Please select a new locale from the locale list");

		}
		bHandled = TRUE;

	} else if( hObjPressed == m_ManagePaths ) {
		// XUI Handles this

	} else if( hObjPressed == m_DownloadIcons ) {
		SETTINGS::getInstance().setDownloadIcon(1 - SETTINGS::getInstance().getDownloadIcon());
		m_DownloadIcons.SetCheck(SETTINGS::getInstance().getDownloadIcon());
		bHandled = TRUE;
	} else if( hObjPressed == m_DownloadBoxart ) {
		SETTINGS::getInstance().setDownloadBoxart(1 - SETTINGS::getInstance().getDownloadBoxart());
		m_DownloadBoxart.SetCheck(SETTINGS::getInstance().getDownloadBoxart());
		bHandled = TRUE;
	} else if( hObjPressed == m_DownloadBanners ) {
		SETTINGS::getInstance().setDownloadBanner(1 - SETTINGS::getInstance().getDownloadBanner());
		m_DownloadBanners.SetCheck(SETTINGS::getInstance().getDownloadBanner());
		bHandled = TRUE;
	} else if( hObjPressed == m_DownloadScreenshots ) {
		SETTINGS::getInstance().setDownloadScreenshots(1 - SETTINGS::getInstance().getDownloadScreenshots());
		m_DownloadScreenshots.SetCheck(SETTINGS::getInstance().getDownloadScreenshots());
		bHandled = TRUE;
	} else if( hObjPressed == m_DownloadBackgrounds ) {
		SETTINGS::getInstance().setDownloadBackground(1 - SETTINGS::getInstance().getDownloadBackground());
		m_DownloadBackgrounds.SetCheck(SETTINGS::getInstance().getDownloadBackground());
		bHandled = TRUE;
	} else if( hObjPressed == m_DownloadVideos ) {
		SETTINGS::getInstance().setDownloadVideo(1 - SETTINGS::getInstance().getDownloadVideo());
		m_DownloadVideos.SetCheck(SETTINGS::getInstance().getDownloadVideo());
		bHandled = TRUE;
	} else if( hObjPressed == m_AutoResumeAssets ) {
		SETTINGS::getInstance().setResumeAssetDownload(1 - SETTINGS::getInstance().getResumeAssetDownload());
		m_AutoResumeAssets.SetCheck(SETTINGS::getInstance().getResumeAssetDownload());
		bHandled = TRUE;
	} else if( hObjPressed == m_DisableAutoScan ) {
		SETTINGS::getInstance().setDisableAutoScan(1- SETTINGS::getInstance().getDisableAutoScan());
		m_DisableAutoScan.SetCheck(SETTINGS::getInstance().getDisableAutoScan());
		bHandled = TRUE;
	} else if( hObjPressed == m_StartManualScan) {
		DebugMsg("ScnOptionsContentSettings", "Start Manual Scan Button pressed.");
		vector<ScanPath> ScanPaths = FSDSql::getInstance().getScanPaths();
		if(ScanPaths.size() > 0) {
			if(ContentManager::getInstance().IsCurrentlyScanning() == FALSE)
			{
				for(unsigned int x = 0; x < ScanPaths.size(); x++)
				{
					ContentManager::getInstance().AddScanPath(ScanPaths.at(x));
				}
			}
			else
			{
				XNotifyQueueUICustom(L"The Scanning Queue is currently busy.  Please try again once scanning has completed.");
			}
		}
		else
		{
			XNotifyQueueUICustom(L"There are currently no defined Scan Paths.  Please add paths and try again.");
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_CoverDownload) {
		if (SETTINGS::getInstance().getCoverCheck())
		  SETTINGS::getInstance().setCoverCheck(false);
		else SETTINGS::getInstance().setCoverCheck(true);
		ContentManager::getInstance().UpdateTimer();
		m_CoverDownload.SetCheck(SETTINGS::getInstance().getCoverCheck());
		bHandled = TRUE;
	} else if( hObjPressed == m_CoverUser ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szUsername);

		// Set Variables to display our Keyboard
		m_dwKeyboardMode = KBMODE_USERNAME;
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szKeyboardUsernameCaption.c_str(), m_szKeyboardUsernamePrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;

	} else if( hObjPressed == m_CoverPass ) {
		memset(&m_kbOverlapped, 0, sizeof(m_kbOverlapped));
		memset(m_kbBuffer, 0, sizeof(m_kbBuffer));

		// Convert our string to a wstring
		m_szTemp = strtowstr(m_szPassword);

		// Set Variables to display our Keyboard
		m_dwKeyboardMode = KBMODE_PASSWORD;
		XShowKeyboardUI(0, VKBD_DEFAULT, m_szTemp.c_str(), m_szKeyboardPasswordCaption.c_str(), m_szKeyboardPasswordPrompt.c_str(),
			m_kbBuffer, 32, &m_kbOverlapped);
		SetTimer(TM_KEYBOARD, 50);
		bHandled = TRUE;
	} else if( hObjPressed == m_CoverLowRes) {
		m_CoverLowRes.SetCheck(TRUE);
		m_CoverMedRes.SetCheck(FALSE);
		m_CoverHighRes.SetCheck(FALSE);
		SETTINGS::getInstance().setCoverQuality(2);
	} else if( hObjPressed == m_CoverMedRes) {
		m_CoverLowRes.SetCheck(FALSE);
		m_CoverMedRes.SetCheck(TRUE);
		m_CoverHighRes.SetCheck(FALSE);
		SETTINGS::getInstance().setCoverQuality(1);
	} else if( hObjPressed == m_CoverHighRes) {
		m_CoverLowRes.SetCheck(FALSE);
		m_CoverMedRes.SetCheck(FALSE);
		m_CoverHighRes.SetCheck(TRUE);
		SETTINGS::getInstance().setCoverQuality(0);
	} else {
		// Button press not recognized

	}

	// Return successfully
	return S_OK;
}

HRESULT CScnOptionsContentSettings::ReadUserSettings( void )
{
	// Set initial state of Cover Downloading Settings Controls
	m_CoverPoll.SetValue(SETTINGS::getInstance().getCoverPoll());
	m_CoverDownload.SetCheck(SETTINGS::getInstance().getCoverCheck());
	wstring NewValue = sprintfaW(L"Current: %d mins", SETTINGS::getInstance().getCoverPoll());
	m_CurrentPoll.SetText(NewValue.c_str());
	m_szUsername = SETTINGS::getInstance().getCoverUser();
	NewValue = strtowstr(m_szUsername);
	if (m_szUsername != "")
		m_CoverUser.SetText(NewValue.c_str());
	m_szPassword = SETTINGS::getInstance().getCoverPass();
	if (m_szPassword != "")
		m_CoverPass.SetText(strtowstr(GetPasswordMask(m_szPassword.length(), "*")).c_str());
	
	// Set initial state of Game List Setting controls
	m_UseBackgrounds.SetCheck(SETTINGS::getInstance().getUseBackground());
	m_UseScreenshots.SetCheck(SETTINGS::getInstance().getUseScreenshots());

	// Set initial state of Marketplace Option controls
	m_DownloadIcons.SetCheck(SETTINGS::getInstance().getDownloadIcon());
	m_DownloadBoxart.SetCheck(SETTINGS::getInstance().getDownloadBoxart());
	m_DownloadBanners.SetCheck(SETTINGS::getInstance().getDownloadBanner());
	m_DownloadScreenshots.SetCheck(SETTINGS::getInstance().getDownloadScreenshots());
	m_DownloadBackgrounds.SetCheck(SETTINGS::getInstance().getDownloadBackground());
	m_DownloadVideos.SetCheck(SETTINGS::getInstance().getDownloadVideo());

	m_ScreenshotSlider.SetValue(SETTINGS::getInstance().getMaxScreenshots());
	wstring szSliderText = sprintfaW(L"%d", SETTINGS::getInstance().getMaxScreenshots());
	m_ScreenshotCountText.SetText(szSliderText.c_str());

	// Set intial state of Game List Visual List
	for( unsigned int nCount = 0; nCount < 3; nCount++ ) {
		m_GameListVisual.SetItemCheck( nCount, FALSE );
	}
	m_GameListVisual.SetItemCheck( SETTINGS::getInstance().getGameListVisual(), TRUE );

	// Set intial state of Game List Transition List
	for( unsigned int nCount = 0; nCount < 3; nCount++ ) {
		m_GameListTransition.SetItemCheck( nCount, FALSE );
	}
	m_GameListTransition.SetItemCheck( SETTINGS::getInstance().getGameListTransition(), TRUE );

	// Set initial state of Marketplace Locale controls
	m_szCurrentLocaleCode = SETTINGS::getInstance().getLocale();
	
	// Fill List Items
	m_LocaleList.InsertItems(0, m_LocaleMap.size() + 1);
	m_LocaleList.SetText(0, L"Select New Locale");
	unsigned int nCount = 1;
	unsigned int nCurSel = 0;
	map<string, string>::iterator iter;
	for( iter = m_LocaleMap.begin(); iter != m_LocaleMap.end(); ++iter) {
		m_LocaleList.SetText( nCount, strtowstr((*iter).first).c_str() );
		DebugMsg("Locale", "Locale Found:  %s", (*iter).first.c_str());
		if( strcmp((*iter).second.c_str(), m_szCurrentLocaleCode.c_str())== 0) {
			m_szCurrentLocale = (*iter).first;
			nCurSel = nCount;
		}
		nCount++;
	}
	// Set the current locale text and some member variables for later use
	m_CurrentLocale.SetText(L"");
	m_CurrentLocale.SetText(strtowstr(m_szCurrentLocale).c_str());
	m_szSelectedLocale = "Select New Locale";
	
	// Set initial state for other stuff here
	m_AutoResumeAssets.SetCheck(SETTINGS::getInstance().getResumeAssetDownload());
	m_DisableAutoScan.SetCheck(SETTINGS::getInstance().getDisableAutoScan());

	// Initalize the check boxes for the cover quality
	switch ( SETTINGS::getInstance().getCoverQuality() ) {
		case 0:		// High Resolution
			m_CoverLowRes.SetCheck(FALSE);
			m_CoverMedRes.SetCheck(FALSE);
			m_CoverHighRes.SetCheck(TRUE);
			break;
		case 1:		// Medium Resolution
			m_CoverLowRes.SetCheck(FALSE);
			m_CoverMedRes.SetCheck(TRUE);
			m_CoverHighRes.SetCheck(FALSE);
			break;
		case 2:		// Low Resolution
			m_CoverLowRes.SetCheck(TRUE);
			m_CoverMedRes.SetCheck(FALSE);
			m_CoverHighRes.SetCheck(FALSE);
			break;
	}
	// *** Future

	return S_OK;
}

HRESULT CScnOptionsContentSettings::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled )
{
	// Handle Vertical Overscan Slider First
	if( hObjSource == m_ScreenshotSlider ) {
		int nOldValue = SETTINGS::getInstance().getMaxScreenshots();
		if(nOldValue < CONTENT_MIN_SCREENSHOTS ) nOldValue = CONTENT_MIN_SCREENSHOTS;
		if(nOldValue > CONTENT_MAX_SCREENSHOTS ) nOldValue = CONTENT_MAX_SCREENSHOTS;

		int nNewValue = pNotifyValueChangedData->nValue;
		if(nNewValue < CONTENT_MIN_SCREENSHOTS ) {
			m_ScreenshotSlider.SetValue( CONTENT_MIN_SCREENSHOTS );
			nNewValue = CONTENT_MIN_SCREENSHOTS;
		}
		if(nNewValue > CONTENT_MAX_SCREENSHOTS ) {
			m_ScreenshotSlider.SetValue( CONTENT_MAX_SCREENSHOTS );
			nNewValue = CONTENT_MAX_SCREENSHOTS;
		}

		wstring ssCountText = sprintfaW(L"%d", nNewValue);
		m_ScreenshotCountText.SetText(ssCountText.c_str());
		if( nOldValue != nNewValue )
			SETTINGS::getInstance().setMaxScreenshots(nNewValue);

		bHandled = TRUE;
	} else if (hObjSource == m_CoverPoll ) {
		int nNewValue = pNotifyValueChangedData->nValue;
		SETTINGS::getInstance().setCoverPoll( nNewValue);
		wstring szTemp = sprintfaW(L"Check: %d mins", nNewValue);
		m_CurrentPoll.SetText(szTemp.c_str());
		ContentManager::getInstance().UpdateTimer();
		bHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScnOptionsContentSettings::InitializeChildren( void )
{	
	// Game List Setting Controls
	GetChildById(L"UseBackgrounds", &m_UseBackgrounds);
	GetChildById(L"UseScreenshots", &m_UseScreenshots);

	// Game List Visual Controls
	GetChildById(L"GameListVisual", &m_GameListVisual);
	GetChildById(L"GameListTransition", &m_GameListTransition);

	// Marketplace Locale
	GetChildById(L"CurrentLocale", &m_CurrentLocale);
	GetChildById(L"LocaleList", &m_LocaleList);
	GetChildById(L"SetLocale", &m_SetLocale);

	// General Buttons
	GetChildById(L"ManagePaths", &m_ManagePaths);

	bool bTemp = false;
	XuiInitControl(L"DownloadIcons", m_DownloadIcons, &bTemp);
	XuiInitControl(L"DownloadBoxart", m_DownloadBoxart, &bTemp);
	XuiInitControl(L"DownloadBanners", m_DownloadBanners, &bTemp);
	XuiInitControl(L"DownloadScreenshots", m_DownloadScreenshots, &bTemp);
	XuiInitControl(L"DownloadBackgrounds", m_DownloadBackgrounds, &bTemp);
	XuiInitControl(L"DownloadVideos", m_DownloadVideos, &bTemp);

	XuiInitControl(L"ScreenshotCounter", m_ScreenshotSlider, &bTemp);
	XuiInitControl(L"ScreenshotCountText", m_ScreenshotCountText, &bTemp);

	XuiInitControl(L"AutoResumeAssets", m_AutoResumeAssets, &bTemp);
	XuiInitControl(L"DisableAutoScan", m_DisableAutoScan, &bTemp);
	XuiInitControl(L"StartManualScan", m_StartManualScan, &bTemp);

	XuiInitControl(L"CoversPollInterval", m_CoverPoll, &bTemp);
	XuiInitControl(L"CoverUsername", m_CoverUser, &bTemp);
	XuiInitControl(L"CoverPassword", m_CoverPass, &bTemp);
	XuiInitControl(L"DisableAutoDownload", m_CoverDownload, &bTemp);
	XuiInitControl(L"CurrentLabel", m_CurrentPoll, &bTemp);
	XuiInitControl(L"UseLowRes", m_CoverLowRes, &bTemp);
	XuiInitControl(L"UseMedRes", m_CoverMedRes, &bTemp);
	XuiInitControl(L"UseHighRes", m_CoverHighRes, &bTemp);
	return S_OK;
}