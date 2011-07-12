#include "stdafx.h"

#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Managers/Skin/SkinManager.h"
#include "../../Tools/Managers/Version/VersionManager.h"

#include "ScnAbout.h"

HRESULT CScnAbout::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	string strBuffer = "";
	InitializeChildren();

	ReadSkinData();
	SetFSDCredits();
	SetThanksCredits();

    return S_OK;
}


//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScnAbout::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{

    return S_OK;
}

HRESULT CScnAbout::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{

    return S_OK;
}

HRESULT CScnAbout::InitializeChildren( void )
{
	HRESULT hr = NULL;	

	hr = GetChildById(L"SkinVersion", &m_SkinVersion);
	hasSkinVersion = hr == S_OK;

	hr = GetChildById(L"DashVersion", &m_XexVersion);
	hasXexVersion = hr == S_OK;

	hr = GetChildById(L"KernelVersion", &m_KernelVersion);
	hasKernelVersion = hr == S_OK;

	hr = GetChildById(L"SkinName", &m_SkinName);
	hasSkinName = hr == S_OK;

	hr = GetChildById(L"MemberList1", &m_MemberList1);
	hasMemberList1 = hr == S_OK;

	hr = GetChildById(L"MemberList2", &m_MemberList2);
	hasMemberList2 = hr == S_OK;

	hr = GetChildById(L"MemberList3", &m_MemberList3);
	hasMemberList3 = hr == S_OK;

	hr = GetChildById(L"ThanksList1", &m_ThanksList1);
	hasThanksList1 = hr == S_OK;

	hr = GetChildById(L"ThanksList2", &m_ThanksList2);
	hasThanksList2 = hr == S_OK;

	hr = GetChildById(L"ThanksList3", &m_ThanksList3);
	hasThanksList3 = hr == S_OK;
	return S_OK;
}

void CScnAbout::ReadSkinData( void )
{
	string strBuffer = "";
	
	if(hasXexVersion)
	{
		m_XexVersion.SetText(strtowstr(VersionManager::getInstance().getFSDDashVersionAsString()).c_str());
	}
	if(hasSkinName)
	{
		m_SkinName.SetText(strtowstr(SETTINGS::getInstance().getSkin()).c_str());
	}

	if(hasSkinVersion)
	{
		m_SkinVersion.SetText(strtowstr(VersionManager::getInstance().getFSDSkinVersionAsString()).c_str());
	}

	if(hasKernelVersion)
	{
		m_KernelVersion.SetText(strtowstr(VersionManager::getInstance().getKernelAsString()).c_str());
	}

}

void CScnAbout::SetFSDCredits( void )
{
	LPWSTR strStringA = L"Anth0ny\nEqUiNoX\nHellDoc";
	LPWSTR strStringB = L"JQE\nkhuong\nMaesterRo";
	LPWSTR strStringC = L"Mattie\nnode21\nWondro";

	if(hasMemberList1)
		m_MemberList1.SetText(strStringA);

	if(hasMemberList2)
		m_MemberList2.SetText(strStringB);

	if(hasMemberList3)
		m_MemberList3.SetText(strStringC);
}

void CScnAbout::SetThanksCredits( void )
{
	LPWSTR strStringA = L"Aut0botK1lla\naxc97c\n[c0z]\nCed2911\nCr4z13\n|Gff|";
	LPWSTR strStringB = L"Lantus\nn3o\nOdb718\nRocky5\nRoofus/Kreet\nSonic-Iso";
	LPWSTR strStringC = L"Team Avalaunch\nTeam XeLove\ntrancy\nTuxuser\nXorloser";

	if(hasThanksList1)
		m_ThanksList1.SetText(strStringA);

	if(hasThanksList2)
		m_ThanksList2.SetText(strStringB);

	if(hasThanksList3)
		m_ThanksList3.SetText(strStringC);
}