#include "stdafx.h"
#include "ScnChooseSkin.h"

#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Settings/Settings.h"

map<string, skinitem> Skins;

HRESULT CScnChooseSkin::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnChooseSkin", "ScnChooseSkin Initialized");

	GetChildById( L"SkinList", &m_SkinList );
	GetChildById( L"PreviewImage", &m_PreviewImage );
	GetChildById( L"CustomizeSkin", &m_customizeSkin );
	GetChildById( L"Back", &m_Back );
	GetChildById( L"Author", &m_Author );
	GetChildById( L"Version", &m_Version );
	GetChildById( L"Title", &m_SkinTitle );
	GetChildById( L"XZP", &m_XZP );
	GetChildById( L"AuthorIcon", &m_AuthorIcon );

	skinChanged = false;
	listLoaded = false;
	activeSkin = 0;
	currentSelected = 0;

	btnReload = new LPCWSTR[2]();
	btnReload[0] = L"Reboot";
	btnReload[1] = L"Reboot Later";

	m_customizeSkin.SetShow(false);
	m_customizeSkin.SetEnable(false);

	LoadSkinList();
	ListInfo(0);

	return S_OK;
}

HRESULT CScnChooseSkin::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_SkinList)
	{
		int Current = m_SkinList.GetCurSel();
		currentSelected = Current;
		for (unsigned int i = 0 ; i < Skins.size() ; i++)
		{
			m_SkinList.SetItemCheck(i,false);
		}
		m_SkinList.SetItemCheck(Current,true);
		std::map<string, skinitem>::iterator itr = Skins.begin();
		for(;itr != Skins.end();itr++)
		{
			if (strcmp(wstrtostr(m_SkinList.GetText(Current)).c_str(), itr->second.SkinName.c_str()) == 0)
			{
				SETTINGS::getInstance().setSkin(itr->first);
				skinChanged = true;
			}
		}
	} else if(hObjPressed == m_customizeSkin) 
	{
		SkinManager::getInstance().setScene("canvasconfig.xur", *this, true);
		bHandled = TRUE;
	} else if (hObjPressed == m_Back)
	{
		if(skinChanged)
		{
			ShowMessageBoxEx(L"XuiMessageBox2", CFreestyleUIApp::getInstance().GetRootObj(), L"Skin Changed", L"Your selected skin has changed. Please restart for changes to take effect.", 2, btnReload, 0, NULL, NULL);
		} else {
			NavigateBack(XUSER_INDEX_ANY);
			bHandled = TRUE;
		}
	}

    return S_OK;
}

HRESULT CScnChooseSkin::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled )
{
	if(hObjSource == m_SkinList)
	{
		ListInfo(pNotifySel->iItem);

		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CScnChooseSkin::OnMsgReturn(XUIMessageMessageBoxReturn *pMsgBox, BOOL &bHandled)
{
	switch( pMsgBox->nButton )
    {
        case 0:
            Restart();
            break;
        case 1:
			NavigateBack(XUSER_INDEX_ANY);
            break;
    }
    
    bHandled = TRUE;

	return S_OK;
}

void CScnChooseSkin::ListInfo(int item)
{
	if(listLoaded)
	{
		std::map<string, skinitem>::iterator itr = Skins.begin();
		for(;itr != Skins.end();itr++)
		{
			if(strcmp((itr->second.SkinName).c_str(),wstrtostr(m_SkinList.GetText(item)).c_str())==0)
			{
				m_Author.SetText(strtowstr(itr->second.Author).c_str());
				m_Version.SetText(strtowstr(itr->second.Version).c_str());
				m_SkinTitle.SetText(strtowstr(itr->second.SkinName).c_str());
				m_PreviewImage.SetImagePath(strtowstr(itr->second.PreviewPath).c_str());
				m_AuthorIcon.SetImagePath(strtowstr(itr->second.IconPath).c_str());
				if(itr->second.isCompressed)
				{
					m_XZP.SetText(L"Yes");
				}
				else
				{
					m_XZP.SetText(L"No");
				}
			}
		}

		if(item == activeSkin && activeSkin == currentSelected)
		{
			m_customizeSkin.SetShow(true);
			m_customizeSkin.SetEnable(true);
		}
		else
		{
			m_customizeSkin.SetShow(false);
			m_customizeSkin.SetEnable(false);
		}
	}
}

void CScnChooseSkin::LoadSkinList()
{
	int Count = m_SkinList.GetItemCount();
	m_SkinList.DeleteItems(0,Count);

	Skins = SkinManager::getInstance().getAvailableSkins();
	m_SkinList.InsertItems(0,Skins.size());
	unsigned int i = 0;
	string currentSkin = SETTINGS::getInstance().getSkin();
	std::map<string, skinitem>::iterator itr = Skins.begin();
	for(;itr != Skins.end();itr++)
	{
		m_SkinList.SetText(i,strtowstr(itr->second.SkinName).c_str());
		
		if (itr->second.SkinFileName == currentSkin)
		{
			activeSkin = i;
			currentSelected = i;
		}
		i++;
	}
	m_SkinList.SetItemCheck(activeSkin,true);
	listLoaded = true;
}