#pragma once

#include <stdafx.h>
#include "ScnGetCredentials.h"
#include "../../Application/FreestyleApp.h"
#include "../../Tools/Managers/SambaClient/SambaClient.h"
#include "../../Tools/Managers/Skin/SkinManager.h"

HRESULT ScnGetCredentials::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	cred = (Cred*)(pInitData->pvInitData);
//	LPCWSTR t = (LPCWSTR)(cred->title);

	InitializeChildren();

	m_title.SetText(L"Enter Samba Credentials");
	
	string message = sprintfaA("Please enter credentials for [%s]", cred->smbPath.c_str());
	m_description.SetText(strtowstr(message).c_str());

	m_login.SetText(L"");
	temp = strtowstr(cred->login);
	m_login.SetText(temp.c_str());
	user = cred->login;

	m_password.SetText(L"");
	temp = strtowstr(cred->password);
	m_password.SetText(temp.c_str());
	password = cred->password;

	m_testResults.SetText(L"Test Connection");
	m_testResults.SetShow(false);

	return S_OK;
}

HRESULT ScnGetCredentials::InitializeChildren()
{
	HRESULT hr;
	hr = GetChildById( L"CredentialsSubScn", &m_ChildScene);
	isNested = hr == S_OK;

	if(isNested)
	{
		m_ChildScene.GetChildById( L"Title_Text", &m_title);
		m_ChildScene.GetChildById( L"MessageText", &m_description);
		m_ChildScene.GetChildById( L"LoginInput", &m_login);
		m_ChildScene.GetChildById( L"PasswordInput", &m_password);
		m_ChildScene.GetChildById( L"ButtonTest", &m_testBtn);
		m_ChildScene.GetChildById( L"TestResults", &m_testResults);
		m_ChildScene.GetChildById( L"XuiSaveCheck", &m_saveCheck);
		m_ChildScene.GetChildById( L"ButtonDone", &m_doneBtn);
	}
	else
	{
		GetChildById( L"Title_Text", &m_title);
		GetChildById( L"MessageText", &m_description);
		GetChildById(L"LoginInput", &m_login);
		GetChildById(L"PasswordInput", &m_password);
		GetChildById(L"ButtonTest", &m_testBtn);
		GetChildById(L"TestResults", &m_testResults);
		GetChildById(L"XuiSaveCheck", &m_saveCheck);
		GetChildById(L"ButtonDone", &m_doneBtn);
	}

	GetChildById(L"Back", &m_back);

	return S_OK;
}

HRESULT ScnGetCredentials::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_back || hObjPressed == m_doneBtn)
	{
		if (hObjPressed == m_doneBtn) {
			// send the messages to the parent
			SambaClient::getInstance().SetSambaCredentials(cred->share, user, password, m_saveCheck.IsChecked() ? true : false);
		}
		Unlink();

		CXuiElement focus;
		focus.Attach(cred->focusItem);
		focus.SetFocus();

		Destroy();

		delete cred;
		bHandled = TRUE;
	}
	else if (hObjPressed == m_login) {
		memset(result,0,sizeof(result));
		memset(&overlapped,0,sizeof(overlapped));

		string message = sprintfaA("Please enter a login (user name) for [%s]", cred->smbPath.c_str());
		wcscpy_s(MessageText,1024,strtowstr(message).c_str());
		bIsPasswordEdit = false;
		temp = strtowstr(user);
		XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,temp.c_str(),m_title.GetText(),MessageText,result,1024,&overlapped);

		SetTimer(TM_KEYBOARD, 250);
	}
	else if (hObjPressed == m_password) {
		memset(result,0,sizeof(result));
		memset(&overlapped,0,sizeof(overlapped));

		string message = sprintfaA("Please enter the password for user [%s] to access [%s]", user.c_str(),cred->smbPath.c_str());
		wcscpy_s(MessageText,1024,strtowstr(message).c_str());
		bIsPasswordEdit = true;
		temp = strtowstr(password);
		XShowKeyboardUI(0,VKBD_DEFAULT|VKBD_HIGHLIGHT_TEXT,temp.c_str(),m_title.GetText(),MessageText,result,1024,&overlapped);

		SetTimer(TM_KEYBOARD, 250);
	}
	else if (hObjPressed == m_testBtn) {

		if (SambaClient::getInstance().TestSambaCredentials(cred->smbPath, user, password)) {
			m_testResults.SetText(L"Success");
		}
		else {
			m_testResults.SetText(L"Failed");
		}
		m_testResults.SetShow(true);
		bHandled = true;
	}
	else if (hObjPressed == m_saveCheck) {
		if (m_saveCheck.IsChecked())
			bSaveToDb = true;
		else
			bSaveToDb = false;
		bHandled = true;
	}
	return S_OK;
}

HRESULT ScnGetCredentials::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch (pTimer->nId)
	{
	case TM_KEYBOARD:
		{
			if (XHasOverlappedIoCompleted(&overlapped))
			{
				if (overlapped.dwExtendedError == ERROR_SUCCESS)
				{
					if (bIsPasswordEdit) {
						if (strcmp(password.c_str(), wstrtostr(result).c_str()) != 0) {
							m_password.SetText(result);
							password = wstrtostr(result);
							m_testResults.SetShow(false);
						}
					}
					else {
						if (strcmp(user.c_str(), wstrtostr(result).c_str()) != 0) {
							m_login.SetText(result);
							user = wstrtostr(result);
							m_testResults.SetShow(false);
						}
					}
				}
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
			}
			break;
		}
	}
	return S_OK;
}



//		GetCredentials(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Delete file", MessageText, 2, btnYesNo, 1, NULL, NULL);


HRESULT GetCredentials(string smbPath, string login, string password, string share) 
{
	CXuiElement p;
	CFreestyleUIApp::getInstance().GetRootObj(&p);

	//if (parent == NULL) {
	//	CFreestyleUIApp::getInstance().GetRootObj(parent);
	//}
	
	//	HXUIOBJ hScene;
	CXuiElement elem;
	HRESULT hResult;

	Cred* c = new Cred();
//	c->title = new WCHAR[wcslen(szTitle)+1];
//    memset( c->title, 0, sizeof(WCHAR) * (wcslen(szTitle) + 1) );
//    memcpy( c->title, szTitle, sizeof(WCHAR) * (wcslen(szTitle) + 1)  );

//	c->description = (LPWSTR)malloc(wcslen(szText)*sizeof(WCHAR) + 2);
//	wcscpy_s(c->description, wcslen(szText), szText);

	c->focusItem = p.GetFocus();
	c->login = login;
	c->password = password;
	c->smbPath = smbPath;
	c->share = share;

	string skinPath = SkinManager::getInstance().getCurrentSkinPath();
	string skinName = SkinManager::getInstance().getCurrentSkinFileName();
	string path;	
	if(SkinManager::getInstance().isCurrentCompressed())
	{
		path = sprintfaA("file://%s%s#",skinPath.c_str(),skinName.c_str());
	}
	else
	{
		path = sprintfaA("file://%s",skinPath.c_str());
	}

	hResult = CXuiScene::SceneCreate( strtowstr(path).c_str(), L"GetCredentials.xur", &elem , (void *)c);

	if (hResult == S_OK) {
		p.AddChild(elem.GetBase());
		elem.EnableInput(true);
		elem.SetShow(true);
		elem.SetFocus();
	}
	else {
		delete c;
	}

	return hResult;
}
