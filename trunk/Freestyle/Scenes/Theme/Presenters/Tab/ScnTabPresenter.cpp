#include "stdafx.h"

#include "../../../../Tools/Settings/Settings.h"
#include "../../../../Tools/Threads/ThreadLock.h"
#include "../../../../Tools/Managers/Theme/TabManager/TabManager.h"

#include "ScnTabPresenter.h"

CScnTabPresenter::CScnTabPresenter(void)
{
	
}

CScnTabPresenter::~CScnTabPresenter(void)
{
	TabManager::getInstance().remove(*this);
}

HRESULT CScnTabPresenter::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	TabManager::getInstance().add(*this);
	InitializeChildren();

	LoadSettings("ScnTabPresenter", *this);

	string mySetting = GetSetting("PRESENTER", "ACTIVE");

	if(strcmp(mySetting.c_str(), "ACTIVE")==0)
		nPresenter = TM_MENU_ACTIVE;
	else if(strcmp(mySetting.c_str(), "PREVIEW")==0)
		nPresenter = TM_MENU_PREVIEW;
	else //if(strcmp(mySettings.c_str(), "LOADED")==0)
		nPresenter = TM_MENU_LOADED;

	if(!hasParentIndex || !hasChildIndex)
		return S_OK;

	thisParent = atoi(wstrtostr(m_ParentIndex.GetText()).c_str());
	thisChild = atoi(wstrtostr(m_ChildIndex.GetText()).c_str());

	UpdateTabProperties(thisParent, thisChild, nPresenter);

	return S_OK;

}

HRESULT CScnTabPresenter::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
    // hObjSource is the control that is getting the focus
    // pNotifyFocusData->hObjOther is the control that is losing focus
	
	if(hObjSource == m_Icon)
	{
		if(hasParentIndex && hasChildIndex)
		{
			TabManager::getInstance().setFocus(thisParent, thisChild);

		}
		bHandled = false;
	}    
    return S_OK;
}

void CScnTabPresenter::RefreshMenu( int nFlag )
{
	if( nFlag == nPresenter )
	{	
		if(!hasParentIndex || !hasChildIndex)
			return;

		UpdateTabProperties( thisParent, thisChild, nFlag );
	}
}

void CScnTabPresenter::UpdateTabProperties(int nParent, int nChild, int nFlag )
{
	//Sleep(10);
	
	if(nFlag != nPresenter)
		return;

	if(!hasParentIndex || !hasChildIndex)
		return;

	
	
	if(thisParent == nParent && thisChild == nChild)
	{		
		while(TabManager::getInstance().isDoneLoading() == false)
		{
			Sleep(0);
		}
		m_lock.Lock();
		TabDataPackage tabData;
		//DebugMsg("TabManager","Requesting Data for %d, %d, %d",thisParent, thisChild,nFlag);
		tabData = TabManager::getInstance().requestTabDataPackage( thisParent, thisChild, nFlag );
	
		int nVisual = 0, nIndex = 0;
		string strPath = ""; 
		if(hasBackground)
		{
			
			nVisual = tabData.ContentBkgVisual;
			nIndex = tabData.ContentBkgIndex;
			
			strPath = TabManager::getInstance().getAssetData(nVisual, nIndex, TM_ASSET_BKG).ImagePath;
			m_Background.SetImagePath(strtowstr(strPath).c_str());
		}
		else
		{
		
	//	DebugMsg("TabManager","No Background");
	
		}

		if(hasIcon)
		{
			nVisual = tabData.ContentIconVisual;
			nIndex = tabData.ContentIconIndex;
			strPath = TabManager::getInstance().getAssetData(nVisual, nIndex, TM_ASSET_ICON).ImagePath;
			m_Icon.SetImage(strtowstr(strPath).c_str());
		}
		if(hasReflection)
		{
			nVisual = tabData.ContentReflectVisual;
			nIndex = tabData.ContentReflectIndex;
			strPath = TabManager::getInstance().getAssetData(nVisual, nIndex, TM_ASSET_REFLECT).ImagePath;
			m_Reflection.SetImagePath(strtowstr(strPath).c_str());
		}
		
		m_lock.Unlock();

		if(hasAnimation)
			m_Animation.SetShow((tabData.ContentAnimState == true) ? TRUE : FALSE);
		if(hasGradient)
			m_Gradient.SetShow((tabData.ContentGradientState) == true ? TRUE : FALSE);
		if(hasIcon){
			if(tabData.ContentIconState == 0)
				m_Icon.SetImage(L"");
		}

		if(hasMainText)
		{
			DWORD dwIdTextColor = 0;
			XUIElementPropVal propvalTextColor;
			XuiObjectGetPropertyId( m_MainText.m_hObj, L"TextColor", &dwIdTextColor );
			DWORD txtColor = tabData.ContentMainTextColor;
			BYTE a = (BYTE) (txtColor >> 24);
			BYTE r = (BYTE) ((txtColor >> 16) & 0xFF);
			BYTE g = (BYTE) ((txtColor >> 8 ) & 0xFF);
			BYTE b = (BYTE) (txtColor & 0xFF);
			float opVal = a / 255.0f;
			a = 0xFF;
			tabData.ContentMainTextColor = D3DCOLOR_ARGB(a, r, g, b);
			propvalTextColor.SetColorVal(tabData.ContentMainTextColor);
			XuiObjectSetProperty( m_MainText.m_hObj, dwIdTextColor, 0, &propvalTextColor );
			m_MainText.SetOpacity(opVal);
		}
		if(hasReflectionText)
		{
			DWORD dwIdTextColor = 0;
			XUIElementPropVal propvalTextColor;
			XuiObjectGetPropertyId( m_ReflectionText.m_hObj, L"TextColor", &dwIdTextColor );
			DWORD txtColor = tabData.ContentReflectTextColor;
			BYTE a = (BYTE) (txtColor >> 24);
			BYTE r = (BYTE) ((txtColor >> 16) & 0xFF);
			BYTE g = (BYTE) ((txtColor >> 8 ) & 0xFF);
			BYTE b = (BYTE) (txtColor & 0xFF);
			float opVal = a / 255.0f;
			a = 0xFF;
			tabData.ContentReflectTextColor = D3DCOLOR_ARGB(a,r,g,b);
			propvalTextColor.SetColorVal(tabData.ContentReflectTextColor);
			XuiObjectSetProperty( m_ReflectionText.m_hObj, dwIdTextColor, 0, &propvalTextColor );
			m_ReflectionText.SetOpacity(opVal);
		}

		this->SetText(strtowstr(tabData.ContentDisplayText).c_str()); 
	}
	else

	{
	//	DebugMsg("TabManager","No Info");
	}
}

HRESULT CScnTabPresenter::InitializeChildren( void )
{
	// Retrieve controls for later use.
	HRESULT hr = NULL;

	hr = GetChildById( L"ParentIndex", &m_ParentIndex );
	hasParentIndex = hr == S_OK;

	hr = GetChildById( L"ChildIndex", &m_ChildIndex );
	hasChildIndex = hr == S_OK;

	hr = GetChildById( L"ContentTab_Background", &m_Background );
	hasBackground = hr == S_OK;

	hr = GetChildById( L"ContentTab_Reflection", &m_Reflection );
	hasReflection = hr == S_OK;

	hr = GetChildById( L"ContentTab_MainText", &m_MainText );
	hasMainText = hr == S_OK;

	hr = GetChildById( L"ContentTab_ReflectionText", &m_ReflectionText );
	hasReflectionText = hr == S_OK;

	hr = GetChildById( L"ContentTab_Icon", &m_Icon );
	hasIcon = hr == S_OK;

	hr = GetChildById( L"ContentTab_Gradient", &m_Gradient );
	hasGradient = hr == S_OK;

	hr = GetChildById( L"ContentTab_Animation", &m_Animation );
	hasAnimation = hr == S_OK;

	return S_OK;
}