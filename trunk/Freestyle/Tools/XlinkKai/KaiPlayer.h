#pragma once
#include "../Texture/TextureItem/TextureItem.h"

class CKaiPlayer
{
public:
	CKaiPlayer()
	{
		// Creating a new buddy item
		m_szPlayerName = L"";
		m_szBuddyDisplayName = L"";
		m_szPlayerAge = L"";
		m_szPlayerLocation = L"";
		m_szPlayerBandwidth = L"";
		m_szCapabilities = L"";
		m_szVector = L"";
		m_szChatMode = L"";
		m_nStatusText = L"";
		m_szPingText = L"";
		m_nStatus = -1;
		m_dwPing = 0;
	}
	CKaiPlayer(string szBuddyName)
	{
		// Creating a new buddy item
		m_szPlayerName = strtowstr(szBuddyName);
		m_szBuddyDisplayName = m_szPlayerName;
		m_szPlayerAge = L"";
		m_szPlayerLocation = L"";
		m_szPlayerBandwidth = L"";
		m_szCapabilities = L"";
		m_szVector = L"";
		m_szChatMode = L"";
		m_nStatusText = L"";
		m_szPingText = L"";
		m_nStatus = -1;
		m_dwPing = 0;
	}
	~CKaiPlayer()
	{
		// Make sure any allocated graphics are released
		BuddyIcon.Release();
		BuddyIcon.ReleaseAll();
	}

	// Methods used to set private variables 
	void SetPlayerName(string szPlayerName)					{ m_szPlayerName = strtowstr(szPlayerName); }
	void SetPlayerAge(string szPlayerAge)					{ m_szPlayerAge = strtowstr(szPlayerAge); }
	void SetPlayerLocation(string szPlayerLocation)			{ m_szPlayerLocation = strtowstr(szPlayerLocation); }
	void SetPlayerBandwidth(string szPlayerBandwidth)		{ m_szPlayerBandwidth = strtowstr(szPlayerBandwidth); }
	void SetPlayerBio(string szPlayerBio)					{ m_szPlayerBio = strtowstr(szPlayerBio); }
	void SetPlayerStatus(int nStatus)						{ m_nStatus = nStatus; }
	void SetPlayerStatusText( string szPlayerStatus )		{ m_nStatusText = strtowstr(szPlayerStatus); }
	void SetPlayerCapabilities(string szCapabilities)		{ m_szCapabilities = strtowstr(szCapabilities); }
	void SetPlayerVector(string szVector)					{ m_szVector = strtowstr(szVector); }
	void SetPlayerChatMode(string szChatMode)				{ m_szChatMode = strtowstr(szChatMode); }
	void SetPlayerPing( DWORD dwPing )						{ m_dwPing = dwPing; m_szPingText = sprintfaW(L"%d", m_dwPing); }
	void SetBuddyOnline( bool bOnline )						{ m_bBuddyOnline = bOnline; }
	void SetBuddyDisplayName( string szDisplayName )		{ m_szBuddyDisplayName = strtowstr(szDisplayName); }

	// Methods to retrieve private variables
	LPCWSTR GetPlayerName( void)							{ return m_szPlayerName.c_str(); }
	LPCWSTR GetPlayerAge( void )							{ return m_szPlayerAge.c_str(); }
	LPCWSTR GetPlayerLocation( void )						{ return m_szPlayerLocation.c_str(); }
	LPCWSTR GetPlayerBandwidth( void )						{ return m_szPlayerBandwidth.c_str(); }
	LPCWSTR GetPlayerBio( void )							{ return m_szPlayerBio.c_str(); }
	int GetPlayerStatus( void )								{ return m_nStatus; }
	LPCWSTR GetPlayerStatusText( void )						{ return m_nStatusText.c_str(); }
	LPCWSTR GetPlayerCapabilities( void )					{ return m_szCapabilities.c_str(); }
	LPCWSTR GetPlayerVector( void )							{ return m_szVector.c_str(); }
	LPCWSTR GetPlayerChatMode( void )						{ return m_szChatMode.c_str(); }
	LPCWSTR GetPlayerPingText( void )							{ return m_szPingText.c_str(); }
	DWORD GetPlayerPing( void )								{ return m_dwPing; }
	LPCWSTR GetBuddyDisplayName( void )						{ return m_szBuddyDisplayName.c_str(); }
	bool GetBuddyOnline( void )								{ return m_bBuddyOnline; }
	
	TextureItem BuddyIcon;
protected:

	// Private Variables
	wstring m_szPlayerName;
	wstring m_szPlayerAge;
	wstring m_szPlayerLocation;
	wstring m_szPlayerBandwidth;
	wstring m_szPlayerBio;
	wstring m_szCapabilities;
	wstring m_szVector;
	wstring m_szChatMode;
	wstring m_nStatusText;
	wstring	m_szPingText;
	int m_nStatus;
	DWORD m_dwPing;
	bool m_bBuddyOnline;
	wstring m_szBuddyDisplayName;
};

	
