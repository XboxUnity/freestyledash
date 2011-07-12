#include "stdafx.h"

#include "ScnPopupWindow.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Settings/Settings.h"

using namespace std;
CScnPopup::~CScnPopup()
{
	DebugMsg("ScnPopupWindow","ScnPopup Destroyed");
}
HRESULT CScnPopup::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	LoadSettings("ScnPopup", *this);
	HRESULT hr = GetChildById( L"ShowButton", &m_ShowButton );
	hasShowButton = hr == S_OK;
	
	hr = GetChildById( L"ToggleButton", &m_ToggleButton );
	hasToggleButton = hr == S_OK;

	hr = GetChildById( L"HideButton", &m_HideButton );
	hasHideButton = hr == S_OK;
	
	string initShown = GetSetting( "INITSHOWN", "FALSE");

	if(initShown == "FALSE")
	{
		isVisible = FALSE;
	}else{
		isVisible = TRUE;
	}

	// If settings determine its to start shown, run the animation
	bool bStartOpen = SETTINGS::getInstance().getShowStatusAtBoot() == 1 ? true : false;

	if(bStartOpen) {
		isVisible = TRUE;
		this->PlayToTransition();
	}

    return S_OK;
}

HRESULT CScnPopup::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	
	if(hasShowButton)
	{
		if(hObjPressed == m_ShowButton)
		{
			if(isVisible == FALSE)
				Show();
			
			bHandled = true;    
		}
	}
	if(hasHideButton)
	{
		if(hObjPressed == m_HideButton)
		{
			if(isVisible == TRUE)
				Hide();
			
			bHandled = true;
		}
	}
	if(hasToggleButton)
	{
		if(hObjPressed == m_ToggleButton)
		{
			if(isVisible == TRUE)
			{
				Hide();
			}else{
				Show();
			}
		
			isVisible = !isVisible;
			bHandled = true;
		}
	}

	return S_OK;   
} 
