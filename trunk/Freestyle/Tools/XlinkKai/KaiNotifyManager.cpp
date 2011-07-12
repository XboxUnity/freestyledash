#include "stdafx.h"
#include "../Generic/xboxtools.h"
#include "../Settings/Settings.h"
#include "KaiNotifyManager.h"

const WCHAR * szContactOnlineMsg = L"%s has logged on Xlink Kai.";
const WCHAR * szContactOfflineMsg = L"%s has logged off Xlink Kai.";
const WCHAR * szInviteToGameMsg = L"%s has invited you to %s";
const WCHAR * szContactPrivateMsg = L"You received a private message from %s.";
const WCHAR * szOpponentPrivateMsg = L"You received a private message from %s.";
const WCHAR * szXlinkKaiOnlineMsg = L"Signed Into Xlink Kai";
const WCHAR * szXlinkKaiOfflineMsg = L"Signed Out of Xlink Kai";

CKaiNotifyManager::CKaiNotifyManager()
{
	UpdateSettings();
}

HRESULT CKaiNotifyManager::UpdateSettings( void )
{
	NotifySettings.NotifyContactOnline = SETTINGS::getInstance().getKaiContactOnlineNotice() == 0 ? false : true;
	NotifySettings.NotifyContactOffline = SETTINGS::getInstance().getKaiContactOfflineNotice() == 0 ? false : true;
	NotifySettings.NotifyInviteToGame = SETTINGS::getInstance().getKaiInviteToGameNotice() == 0 ? false : true;
	NotifySettings.NotifyReceivedContactPM = SETTINGS::getInstance().getKaiContactPMNotice() == 0 ? false : true;
	NotifySettings.NotifyReceivedOpponentPM = SETTINGS::getInstance().getKaiOpponentPMNotice() == 0 ? false : true;

	return S_OK;
}

HRESULT CKaiNotifyManager::NotifyXlinkKaiOnline(string aUser)
{
	wstring szMessage = sprintfaW(szXlinkKaiOnlineMsg, strtowstr(aUser).c_str());

	return SendUserNotice(szMessage);
}

HRESULT CKaiNotifyManager::NotifyXlinkKaiOffline(string aUser)
{
	wstring szMessage = sprintfaW(szXlinkKaiOfflineMsg, strtowstr(aUser).c_str());

	return SendUserNotice(szMessage);
}

HRESULT CKaiNotifyManager::NotifyContactOnline(string aContact)
{
	if(NotifySettings.NotifyContactOnline == false)
		return S_FALSE;

	wstring szMessage = sprintfaW(szContactOnlineMsg, strtowstr(aContact).c_str());

	return SendUserNotice(szMessage);
}
HRESULT CKaiNotifyManager::NotifyContactOffline(string aContact)
{
	if(NotifySettings.NotifyContactOffline == false)
		return S_FALSE;

	wstring szMessage = sprintfaW(szContactOfflineMsg, strtowstr(aContact).c_str());

	return SendUserNotice(szMessage);
}
HRESULT CKaiNotifyManager::NotifyInviteToGame(string aContact, string aVector)
{
	if(NotifySettings.NotifyInviteToGame == false)
		return S_FALSE;

	wstring szMessage = sprintfaW(szInviteToGameMsg, strtowstr(aContact).c_str(), strtowstr(aVector).c_str());

	return SendUserNotice(szMessage);
}
HRESULT CKaiNotifyManager::NotifyReceivedContactPM(string aContact, string aMessage)
{
	if(NotifySettings.NotifyReceivedContactPM == false)
		return S_FALSE;

	wstring szMessage = sprintfaW(szContactPrivateMsg, strtowstr(aContact).c_str());

	return SendUserNotice(szMessage);
}
HRESULT CKaiNotifyManager::NotifyReceivedOpponentPM(string aOpponent, string aMessage)
{
	if(NotifySettings.NotifyReceivedOpponentPM == false)
		return S_FALSE;

	wstring szMessage = sprintfaW(szOpponentPrivateMsg, strtowstr(aOpponent).c_str());

	return SendUserNotice(szMessage);
}
HRESULT CKaiNotifyManager::SendUserNotice(wstring szDisplayString)
{
	DebugMsg("NoticeManager", "String:  %s", wstrtostr(szDisplayString).c_str());
	return XNotifyQueueUICustom((WCHAR*)szDisplayString.c_str());
}