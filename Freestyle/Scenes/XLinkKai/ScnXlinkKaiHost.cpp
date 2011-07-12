#include "stdafx.h"

#include "ScnXlinkKaiHost.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/XlinkKai/KaiMessage.h"
#include "../../Tools/XlinkKai/General/KaiManager.h"

HRESULT CScnXlinkKaiHost::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	DebugMsg("CScnXlinkKaiHost","CScnXlinkKaiHost::OnInit");

	HRESULT hr = GetChildById( L"KaiControls", &m_KaiControls );
	isNested = hr == S_OK;

	DebugMsg("CScnXlinkKaiHost","IsNested %d", isNested);

	if(isNested)
	{
		m_KaiControls.GetChildById( L"PrivateArena", &m_PrivateArena );
		m_KaiControls.GetChildById( L"AddPlayer", &m_AddPlayer );
		m_KaiControls.GetChildById( L"SubtractPlayer", &m_SubtractPlayer );
		m_KaiControls.GetChildById( L"OK", &m_OK );
		m_KaiControls.GetChildById( L"Password", &m_Password );
		m_KaiControls.GetChildById( L"Description", &m_Description );
		m_KaiControls.GetChildById( L"PlayerLimit", &m_PlayerLimit );
	}
	else
	{
		GetChildById( L"PrivateArena", &m_PrivateArena );
		GetChildById( L"AddPlayer", &m_AddPlayer );
		GetChildById( L"SubtractPlayer", &m_SubtractPlayer );
		GetChildById( L"OK", &m_OK );
		GetChildById( L"Password", &m_Password );
		GetChildById( L"Description", &m_Description );
		GetChildById( L"PlayerLimit", &m_PlayerLimit );
	}

	GetChildById( L"Back", &m_back );

	buffer = new WCHAR[512];

	enabled = false;
	EnableControls(false);
	maxPlayers = 16;
	m_PlayerLimit.SetText(L"16");

    return S_OK;
}

HRESULT CScnXlinkKaiHost::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_AddPlayer)
	{	
		if(maxPlayers < 32)
		{
			maxPlayers++;
		}
		m_PlayerLimit.SetText(strtowstr(sprintfaA("%d",maxPlayers)).c_str());

		bHandled = TRUE;
	}
	else if (hObjPressed == m_SubtractPlayer)
	{
		if(maxPlayers > 0)
		{
			maxPlayers--;
		}
		m_PlayerLimit.SetText(strtowstr(sprintfaA("%d",maxPlayers)).c_str());

		bHandled = TRUE;
	}
	else if (hObjPressed == m_OK)
	{
		if(!enabled)
		{
			KaiManager::getInstance().KaiHost();
		}
		else
		{
			KaiManager::getInstance().KaiHostArena(password, description, maxPlayers);
		}

		NavigateBack(XUSER_INDEX_ANY);
		
		bHandled = TRUE;
	}
	else if (hObjPressed == m_Password)
	{	
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));

		temp = strtowstr(password);

		XShowKeyboardUI(0,VKBD_DEFAULT,temp.c_str(),L"Arena Password",L"Enter a password",buffer,32,&keyboard);
		SetTimer(TM_KEYBOARD, 50);

		inputType = 1;

		bHandled = TRUE;
	}
	else if (hObjPressed == m_Description)
	{
		memset(&keyboard, 0, sizeof(keyboard));
		memset(buffer, 0, sizeof(buffer));

		temp = strtowstr(description);

		XShowKeyboardUI(0,VKBD_DEFAULT,temp.c_str(),L"Arena Description",L"Enter a brief description",buffer,200,&keyboard);
		SetTimer(TM_KEYBOARD, 50);

		inputType = 2;

		bHandled = TRUE;
	}
	else if (hObjPressed == m_PrivateArena)
	{
		if(!enabled)
		{
			m_PrivateArena.SetCheck(true);
			EnableControls(true);

			enabled = true;
		}
		else
		{
			m_PrivateArena.SetCheck(false);
			EnableControls(false);

			enabled = false;
		}

		bHandled = TRUE;
	}
	else if (hObjPressed == m_back)
	{
		NavigateBack(XUSER_INDEX_ANY);

		bHandled = TRUE;
	}

    return S_OK;
}

HRESULT CScnXlinkKaiHost::OnTimer(XUIMessageTimer *pTimer, BOOL& bHandled)
{
    switch(pTimer->nId)
    {
		case TM_KEYBOARD:
		{
			if(XHasOverlappedIoCompleted(&keyboard))
            {
				KillTimer(TM_KEYBOARD);
				bHandled = TRUE;
				if(keyboard.dwExtendedError == ERROR_SUCCESS)
                {
					if(inputType == 1)
					{
						password = wstrtostr(buffer);
						m_Password.SetText(strtowstr(PasswordMask(password.length())).c_str());
					}
					else if(inputType == 2)
					{
						m_Description.SetText(buffer);
						description = wstrtostr(buffer);
					}
				}				
			}
			break;
		}
    }
    return S_OK;
}

HRESULT CScnXlinkKaiHost::EnableControls(bool toggle)
{
	m_AddPlayer.SetEnable(toggle);
	m_SubtractPlayer.SetEnable(toggle);
	m_Password.SetEnable(toggle);
	m_Description.SetEnable(toggle);

	return S_OK;
}

string CScnXlinkKaiHost::PasswordMask(int size)
{
	string temp = "";

	for(int i=0; i<size; i++)
	{
		temp.append("*");
	}

	return temp;
}