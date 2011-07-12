#include "stdafx.h"

#include "../../../../Tools/Settings/Settings.h"
#include "../../../../Tools/Threads/ThreadLock.h"
#include "../../../../Tools/Managers/Theme/TabManager/TabManager.h"

#include "TabConfigHub.h"
#include "ScnTabConfig.h"

CScnTabConfig::CScnTabConfig()
{
	//Constructor
}

CScnTabConfig::~CScnTabConfig()
{
	//Deconstructor
	TabConfigHub::getInstance().remove(*this);


}

void CScnTabConfig::UpdateSelectedStats(TabDataPackage tabData)
{
	int nVisual;
	string strBuffer;

	if(hasBkgVisualStat)
	{
		nVisual = tabData.ContentBkgVisual;
		strBuffer = TabManager::getInstance().getVisualData(nVisual).VisualName;

		m_BkgVisualStat.SetText(strtowstr(strBuffer).c_str());
	}
	if(hasIconVisualStat)
	{
		nVisual = tabData.ContentIconVisual;
		strBuffer = TabManager::getInstance().getVisualData(nVisual).VisualName;

		m_IconVisualStat.SetText(strtowstr(strBuffer).c_str());
	}
	if(hasReflectVisualStat)
	{
		nVisual = tabData.ContentReflectVisual;
		strBuffer = TabManager::getInstance().getVisualData(nVisual).VisualName;

		m_ReflectVisualStat.SetText(strtowstr(strBuffer).c_str());
	}
	if(hasAnimStateStat)
	{
		strBuffer = (tabData.ContentAnimState) ? "Visible" : "Hidden";
		m_AnimStateStat.SetText(strtowstr(strBuffer).c_str());
	}
	if(hasGradientStateStat)
	{
		strBuffer = (tabData.ContentGradientState) ? "Visible" : "Hidden";
		m_GradientStateStat.SetText(strtowstr(strBuffer).c_str());
	}
	if(hasIconStateStat)
	{
		strBuffer = (tabData.ContentIconState) ? "Visible" : "Hidden";
		m_IconStateStat.SetText(strtowstr(strBuffer).c_str());
	}
	DWORD color = 0;
	if(hasReflectTextColorStat)
	{
		color = tabData.ContentReflectTextColor;
		BYTE a = (BYTE) (color >> 24);
		BYTE r = (BYTE) ((color >> 16) & 0xFF);
		BYTE g = (BYTE) ((color >> 8 ) & 0xFF);
		BYTE b = (BYTE) (color & 0xFF);
		strBuffer = sprintfaA("(%d, %d, %d, %d)", a,r,g,b);
		m_ReflectTextColorStat.SetText(strtowstr(strBuffer).c_str());
	}
	strBuffer = "";
	color = 0;
	if(hasMainTextColorStat)
	{
		color = tabData.ContentMainTextColor;
		BYTE a = (BYTE) (color >> 24);
		BYTE r = (BYTE) ((color >> 16) & 0xFF);
		BYTE g = (BYTE) ((color >> 8 ) & 0xFF);
		BYTE b = (BYTE) (color & 0xFF);
		strBuffer = sprintfaA("(%d, %d, %d, %d)", a,r,g,b);
		m_MainTextColorStat.SetText(strtowstr(strBuffer).c_str());
	}
}


HRESULT CScnTabConfig::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	InitializeChildren();

	focusParent = TabManager::getInstance().getFocusParent();
	focusChild = TabManager::getInstance().getFocusChild();

	TabDataPackage tabData = TabManager::getInstance().requestTabDataPackage(focusParent, focusChild, TM_MENU_PREVIEW);

	TabConfigHub::getInstance().setPreviewData(tabData);

	CreateInitialPreview();

	FillBackgroundList();
	FillIconList();
	FillReflectionList();

	SetCurrentItems();

	TabConfigHub::getInstance().add(*this);

	return S_OK;
}

HRESULT CScnTabConfig::OnNotifySelchanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled )
{

	if(hObjSource == m_BkgList)
	{
		int nSel = pNotifySel->iItem;
		
		int nIndex = TabConfigHub::getInstance().getPreviewData().ContentBkgIndex;

		VisualData visData = TabManager::getInstance().getVisualData(nSel);

		if(hasBkgListCounter)
		{
		
			string strBuffer = sprintfaA("%d", nSel+1);
			m_BkgListCounter.SetText(strtowstr(strBuffer).c_str());
		}

		if(hasVisualName)
		{
			m_VisualName.SetText(strtowstr(visData.VisualName).c_str());
		}

		if(hasVisualDescription)
		{
			m_VisualDescription.SetText(strtowstr(visData.VisualDescription).c_str());
		}
		
		AssetData asset = TabManager::getInstance().getAssetData(nSel, nIndex, TM_ASSET_BKG);
		string strName = asset.Name;
		
		if(hasBkgName)
			m_BkgName.SetText(strtowstr(strName).c_str());

		string strPreviewPath = asset.ImagePath;
		


		m_Lock.Lock();
		m_BkgPreview.SetImagePath(L"");
		m_BkgPreview.SetImagePath(strtowstr(strPreviewPath).c_str());
		m_Lock.Unlock();
	}
	if(hObjSource == m_IconList)
	{
		int nSel = pNotifySel->iItem;

		int nIndex = TabConfigHub::getInstance().getPreviewData().ContentIconIndex;

		VisualData visData = TabManager::getInstance().getVisualData(nSel);

		if(hasIconListCounter)
		{
		
			string strBuffer = sprintfaA("%d", nSel+1);
			m_IconListCounter.SetText(strtowstr(strBuffer).c_str());
		} 

		if(hasVisualName)
		{
			m_VisualName.SetText(strtowstr(visData.VisualName).c_str());
		}

		if(hasVisualDescription)
		{
			m_VisualDescription.SetText(strtowstr(visData.VisualDescription).c_str());
		}
		AssetData asset = TabManager::getInstance().getAssetData(nSel, nIndex, TM_ASSET_ICON);
		string strName = asset.Name;
		
		if(hasIconName)
			m_IconName.SetText(strtowstr(strName).c_str());

		string strPreviewPath = asset.ImagePath;
		
		m_Lock.Lock();
		m_IconPreview.SetImagePath(L"");
		m_IconPreview.SetImagePath(strtowstr(strPreviewPath).c_str());
		m_Lock.Unlock();
	}

	if(hObjSource == m_ReflectionList)
	{
		int nSel = pNotifySel->iItem;

		int nIndex = TabConfigHub::getInstance().getPreviewData().ContentReflectIndex;

		VisualData visData = TabManager::getInstance().getVisualData(nSel);

		if(hasReflectionListCounter)
		{
			string strBuffer = sprintfaA("%d", nSel+1);
			m_ReflectionListCounter.SetText(strtowstr(strBuffer).c_str());
		} 

		if(hasVisualName)
		{
			m_VisualName.SetText(strtowstr(visData.VisualName).c_str());
		}

		if(hasVisualDescription)
		{
			m_VisualDescription.SetText(strtowstr(visData.VisualDescription).c_str());
		}
		AssetData asset = TabManager::getInstance().getAssetData(nSel, nIndex, TM_ASSET_REFLECT);
		string strName = asset.Name;
		
		if(hasReflectionName)
			m_ReflectionName.SetText(strtowstr(strName).c_str());

		string strPreviewPath = asset.ImagePath;
		
		m_Lock.Lock();
		m_ReflectionPreview.SetImagePath(L"");
		m_ReflectionPreview.SetImagePath(strtowstr(strPreviewPath).c_str());
		m_Lock.Unlock();
	}

	return S_OK;

}


HRESULT CScnTabConfig::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled ) 
{

	if(hObjPressed == m_SaveChanges)
	{
		TabManager::getInstance().SaveXML_MenuSettings(TM_MENU_PREVIEW);
		TabManager::getInstance().copyMenuMap(TM_MENU_PREVIEW, TM_MENU_ACTIVE, true);
	}
	if(hObjPressed == m_ApplyTabChange)
	{
		int nParent = TabManager::getInstance().getFocusParent();
		int nChild = TabManager::getInstance().getFocusChild();

		TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();

		TabManager::getInstance().submitTabDataPackage(nParent, nChild, tabData, TM_MENU_PREVIEW);
	}

	if(hObjPressed == m_ApplyToMenu)
	{

		int focusParent = TabManager::getInstance().getFocusParent();
		int focusChild = TabManager::getInstance().getFocusChild();

		TabDataPackage tabDataSrc = TabManager::getInstance().requestTabDataPackage( focusParent, focusChild, TM_MENU_PREVIEW );
		TabDataPackage tabDataDest;

		int myChildSize = TabManager::getInstance().getChildCount(focusParent, TM_MENU_PREVIEW);
		
		for(int x = 0; x < myChildSize; x++)
		{
		
			tabDataDest = TabManager::getInstance().requestTabDataPackage(focusParent, x, TM_MENU_PREVIEW);

			tabDataSrc.ContentBkgIndex = tabDataDest.ContentBkgIndex;
			tabDataSrc.ContentIconState = tabDataDest.ContentIconState;
			tabDataSrc.ContentIconIndex = tabDataDest.ContentIconIndex;
			tabDataSrc.ContentReflectIndex = tabDataDest.ContentReflectIndex;
			tabDataSrc.ContentDisplayText = tabDataDest.ContentDisplayText;

			TabManager::getInstance().submitTabDataPackage(focusParent, x, tabDataSrc, TM_MENU_PREVIEW);
		}
	}

	if(hObjPressed == m_ApplyToDash)
	{
		int focusParent = TabManager::getInstance().getFocusParent();
		int focusChild = TabManager::getInstance().getFocusChild();

		TabDataPackage tabDataSrc = TabManager::getInstance().requestTabDataPackage( focusParent, focusChild, TM_MENU_PREVIEW );
		TabDataPackage tabDataDest;

		int myParentSize = TabManager::getInstance().getParentCount(TM_MENU_PREVIEW);
		int myChildSize = 0;

		for(int x = 0; x < myParentSize; x++)
		{
			myChildSize = TabManager::getInstance().getChildCount(x, TM_MENU_PREVIEW);
			for(int y = 0; y < myChildSize; y++)
			{
				tabDataDest = TabManager::getInstance().requestTabDataPackage(x, y, TM_MENU_PREVIEW);
	
				tabDataSrc.ContentBkgIndex = tabDataDest.ContentBkgIndex;
				tabDataSrc.ContentIconState = tabDataDest.ContentIconState;
				tabDataSrc.ContentIconIndex = tabDataDest.ContentIconIndex;
				tabDataSrc.ContentReflectIndex = tabDataDest.ContentReflectIndex;
				tabDataSrc.ContentDisplayText = tabDataDest.ContentDisplayText;

				TabManager::getInstance().submitTabDataPackage(x, y, tabDataSrc, TM_MENU_PREVIEW);
			}
		}
	}

	if(hObjPressed == m_Randomize)
	{
		int nVisualCount =  TabManager::getInstance().getVisualCount();
		int myChildSize = 0;
		TabDataPackage tabData;
		VisualData visData;
		
		if(hasRandomStatus)
			m_RandomStatus.SetShow(true);

		int myParentSize = TabManager::getInstance().getParentCount(TM_MENU_PREVIEW);

		for(int x = 0; x < myParentSize; x++)
		{
			myChildSize = TabManager::getInstance().getChildCount(x, TM_MENU_PREVIEW);
			for(int y = 0; y < myChildSize; y++)
			{	
				int nAsset[3] = {0, 0, 0};
				int nMaxAsset[3] = {0, 0, 0};
				
				for(int i = 0; i < 3; i++)
				{
					while(nMaxAsset[i] == 0)
					{
						nAsset[i] = rand() % nVisualCount;
						if(i == 0)
							nMaxAsset[0] = TabManager::getInstance().getVisualData(nAsset[0]).nBkgListSize;

						if(i == 1)
							nMaxAsset[1] = TabManager::getInstance().getVisualData(nAsset[1]).nIconListSize;

						if(i == 2)
							nMaxAsset[2] = TabManager::getInstance().getVisualData(nAsset[2]).nReflectListSize;
					}
				}
				
				int nRed = rand() % 255;
				int nGreen = rand() % 255;
				int nBlue = rand() % 255;

				tabData = TabManager::getInstance().requestTabDataPackage(x, y, TM_MENU_PREVIEW);
				
				tabData.ContentBkgVisual = nAsset[0];
				tabData.ContentIconVisual = nAsset[1];
				tabData.ContentReflectVisual = nAsset[2];
				tabData.ContentMainTextColor = D3DCOLOR_ARGB(255, nRed, nGreen, nBlue);
				tabData.ContentReflectTextColor = D3DCOLOR_ARGB(255, nRed, nGreen, nBlue);

				TabManager::getInstance().submitTabDataPackage(x, y, tabData, TM_MENU_PREVIEW);
			}
		}

		if(hasRandomStatus)
			m_RandomStatus.SetShow(false);
	}

	if(hObjPressed == m_SelectTab)
	{
		int nParent = TabManager::getInstance().getFocusParent();
		int nChild = TabManager::getInstance().getFocusChild();

		TabDataPackage tabData = TabManager::getInstance().requestTabDataPackage(nParent, nChild, TM_MENU_PREVIEW);
		
		TabConfigHub::getInstance().setPreviewData(tabData);
	}
	if(hObjPressed == m_BkgList)
	{
		int nCurSel = m_BkgList.GetCurSel();
		for(int i = 0; i < m_BkgList.GetItemCount(); i++)
		{
			m_BkgList.SetItemCheck(i, false);
		}

		m_BkgList.SetItemCheck(nCurSel, true);

		TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();

		tabData.ContentBkgVisual = nCurSel;

		// Update Preview Tab
		TabConfigHub::getInstance().setPreviewImage(tabData);
	}

	if(hObjPressed == m_IconList)
	{
		int nCurSel = m_IconList.GetCurSel();
		for(int i = 0; i < m_IconList.GetItemCount(); i++)
		{
			m_IconList.SetItemCheck(i, false);
		}

		m_IconList.SetItemCheck(nCurSel, true);

		TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();

		tabData.ContentIconVisual = nCurSel;

		// Update Preview Tab
		TabConfigHub::getInstance().setPreviewImage(tabData);
	}

	if(hObjPressed == m_ReflectionList)
	{
		int nCurSel = m_ReflectionList.GetCurSel();
		for(int i = 0; i < m_ReflectionList.GetItemCount(); i++)
		{
			m_ReflectionList.SetItemCheck(i, false);
		}

		m_ReflectionList.SetItemCheck(nCurSel, true);

		TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();

		tabData.ContentReflectVisual = nCurSel;

		// Update Preview Tab
		TabConfigHub::getInstance().setPreviewImage(tabData);
	}

	return S_OK;

}

HRESULT CScnTabConfig::OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL& bHandled)
{


    if(hObjSource == m_MainRed || hObjSource == m_MainGreen || hObjSource == m_MainBlue || hObjSource == m_MainAlpha)
	{
		int newValue = pNotifyValueChangedData->nValue;
		string strValue = sprintfaA("%d", newValue);
		int nFlag = 0;

		if(hObjSource == m_MainRed && hasMainRedValue){
			nFlag = TM_COLOR_RED;
			m_MainRedValue.SetText(strtowstr(strValue).c_str());
		}

		if(hObjSource == m_MainGreen && hasMainGreenValue) {
			nFlag = TM_COLOR_GREEN;
			m_MainGreenValue.SetText(strtowstr(strValue).c_str());
		}

		if(hObjSource == m_MainBlue && hasMainBlueValue){
			nFlag = TM_COLOR_BLUE;
			m_MainBlueValue.SetText(strtowstr(strValue).c_str());
		}

		if(hObjSource == m_MainAlpha && hasMainAlphaValue) {
			nFlag = TM_COLOR_ALPHA;
			m_MainAlphaValue.SetText(strtowstr(strValue).c_str());
		}

		ChangeMainColor((BYTE)newValue, nFlag);

	}

    if(hObjSource == m_ReflectRed || hObjSource == m_ReflectGreen || hObjSource == m_ReflectBlue || hObjSource == m_ReflectAlpha)
	{
		int newValue = pNotifyValueChangedData->nValue;
		string strValue = sprintfaA("%d", newValue);
		int nFlag = 0;

		if(hObjSource == m_ReflectRed && hasReflectRedValue){
			nFlag = TM_COLOR_RED;
			m_ReflectRedValue.SetText(strtowstr(strValue).c_str());
		}

		if(hObjSource == m_ReflectGreen && hasReflectGreenValue) {
			nFlag = TM_COLOR_GREEN;
			m_ReflectGreenValue.SetText(strtowstr(strValue).c_str());
		}

		if(hObjSource == m_ReflectBlue && hasReflectBlueValue){
			nFlag = TM_COLOR_BLUE;
			m_ReflectBlueValue.SetText(strtowstr(strValue).c_str());
		}

		if(hObjSource == m_ReflectAlpha && hasReflectAlphaValue) {
			nFlag = TM_COLOR_ALPHA;
			m_ReflectAlphaValue.SetText(strtowstr(strValue).c_str());
		}

		ChangeReflectColor((BYTE)newValue, nFlag);

	}

	bHandled = TRUE;

    return S_OK;
}

void CScnTabConfig::ChangeMainColor(BYTE value, int nFlag)
{

		BYTE a, r, g, b;
		DWORD txtColor;

		
		TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();
	
		txtColor = tabData.ContentMainTextColor;

		a = (BYTE) (txtColor >> 24);
		r = (BYTE) ((txtColor >> 16) & 0xFF);
		g = (BYTE) ((txtColor >> 8 ) & 0xFF);
		b = (BYTE) (txtColor & 0xFF);
		
		if(nFlag == TM_COLOR_ALPHA)
			a = value;
		if(nFlag == TM_COLOR_RED)
			r = value;
		if(nFlag == TM_COLOR_GREEN)
			g = value;
		if(nFlag == TM_COLOR_BLUE)
			b = value;

		txtColor = D3DCOLOR_ARGB(a, r, g, b);
		tabData.ContentMainTextColor = txtColor;

		// Method to update the preview image
		TabConfigHub::getInstance().setPreviewImage(tabData);


}

void CScnTabConfig::ChangeReflectColor(BYTE value, int nFlag)
{

		BYTE a, r, g, b;
		DWORD txtColor;

		TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();

		txtColor = tabData.ContentReflectTextColor;

		a = (BYTE) (txtColor >> 24);
		r = (BYTE) ((txtColor >> 16) & 0xFF);
		g = (BYTE) ((txtColor >> 8 ) & 0xFF);
		b = (BYTE) (txtColor & 0xFF);
		
		if(nFlag == TM_COLOR_ALPHA)
			a = value;
		if(nFlag == TM_COLOR_RED)
			r = value;
		if(nFlag == TM_COLOR_GREEN)
			g = value;
		if(nFlag == TM_COLOR_BLUE)
			b = value;

		txtColor = D3DCOLOR_ARGB(a, r, g, b);
		
		tabData.ContentReflectTextColor = txtColor;

		
		// Method to update Preview Tab
		TabConfigHub::getInstance().setPreviewImage(tabData);



}

void CScnTabConfig::CreateInitialPreview()
{
	

	TabDataPackage tabData;

	tabData = TabConfigHub::getInstance().getPreviewData();

	UpdateTabPreview(tabData);
	UpdateSelectedStats(tabData);
}

void CScnTabConfig::FillReflectionList()
{

	if(!hasReflectionList)
		return;
	
	int nVisualCount = TabManager::getInstance().getVisualCount();

	if(hasReflectionListCounterTotal)
	{
		m_ReflectionListCounterTotal.SetText(strtowstr(sprintfaA("of %d", nVisualCount)).c_str());
	}

	VisualData * visData = new VisualData[nVisualCount];

	m_ReflectionList.InsertItems(0, nVisualCount);

	for(int i = 0; i < nVisualCount; i++)
	{
		visData[i] = TabManager::getInstance().getVisualData(i);

		m_ReflectionList.SetText(i, strtowstr(visData[i].VisualName).c_str());
	}

	delete [] visData;
	visData = NULL;
}

void CScnTabConfig::FillIconList()
{

	if(!hasIconList)
		return;
	
	int nVisualCount = TabManager::getInstance().getVisualCount();

	if(hasIconListCounterTotal)
	{
		m_IconListCounterTotal.SetText(strtowstr(sprintfaA("of %d", nVisualCount)).c_str());
	}

	VisualData * visData = new VisualData[nVisualCount];

	m_IconList.InsertItems(0, nVisualCount);

	for(int i = 0; i < nVisualCount; i++)
	{
		visData[i] = TabManager::getInstance().getVisualData(i);

		m_IconList.SetText(i, strtowstr(visData[i].VisualName).c_str());
	}

	delete [] visData;
	visData = NULL;
}

void CScnTabConfig::FillBackgroundList()
{

	if(!hasBkgList)
		return;
	
	int nVisualCount = TabManager::getInstance().getVisualCount();

	if(hasBkgListCounterTotal)
	{
		m_BkgListCounterTotal.SetText(strtowstr(sprintfaA("of %d", nVisualCount)).c_str());
	}

	VisualData * visData = new VisualData[nVisualCount];

	m_BkgList.InsertItems(0, nVisualCount);

	for(int i = 0; i < nVisualCount; i++)
	{
		visData[i] = TabManager::getInstance().getVisualData(i);

		m_BkgList.SetText(i, strtowstr(visData[i].VisualName).c_str());
	}

	delete [] visData;
	visData = NULL;
}

void CScnTabConfig::InitializeChildren()
{
	HRESULT hr = NULL;

	// Background Controls
	hr = GetChildById(L"BkgVisualList", &m_BkgList);
	hasBkgList = hr == S_OK;
	hr = GetChildById(L"BkgPreviewImage", &m_BkgPreview);
	hasBkgPreview = hr == S_OK;
	hr = GetChildById(L"BackgroundName", &m_BkgName);
	hasBkgName = hr == S_OK;
	hr = GetChildById(L"BkgListCounter", &m_BkgListCounter);
	hasBkgListCounter = hr == S_OK;
	hr = GetChildById(L"BkgListCounterTotal", &m_BkgListCounterTotal);
	hasBkgListCounterTotal = hr == S_OK;

	// Icon Controls
	hr = GetChildById(L"IconVisualList", &m_IconList);
	hasIconList = hr == S_OK;
	hr = GetChildById(L"IconPreviewImage", &m_IconPreview);
	hasIconPreview = hr == S_OK;
	hr = GetChildById(L"IconName", &m_IconName);
	hasIconName = hr == S_OK;
	hr = GetChildById(L"IconListCounter", &m_IconListCounter);
	hasIconListCounter = hr == S_OK;
	hr = GetChildById(L"IconListCounterTotal", &m_IconListCounterTotal);
	hasIconListCounterTotal = hr == S_OK;

	// Reflection Controls
	hr = GetChildById(L"ReflectionVisualList", &m_ReflectionList);
	hasReflectionList = hr == S_OK;
	hr = GetChildById(L"ReflectionPreviewImage", &m_ReflectionPreview);
	hasReflectionPreview = hr == S_OK;
	hr = GetChildById(L"ReflectionName", &m_ReflectionName);
	hasReflectionName = hr == S_OK;
	hr = GetChildById(L"ReflectionListCounter", &m_ReflectionListCounter);
	hasReflectionListCounter = hr == S_OK;
	hr = GetChildById(L"ReflectionListCounterTotal", &m_ReflectionListCounterTotal);
	hasReflectionListCounterTotal = hr == S_OK;

	// Text Color Controls
	hr = GetChildById(L"MainRedValue", &m_MainRedValue);
	hasMainRedValue = hr == S_OK;
	hr = GetChildById(L"MainGreenValue", &m_MainGreenValue);
	hasMainGreenValue = hr == S_OK;
	hr = GetChildById(L"MainBlueValue", &m_MainBlueValue);
	hasMainBlueValue = hr == S_OK;
	hr = GetChildById(L"MainAlphaValue", &m_MainAlphaValue);
	hasMainAlphaValue = hr == S_OK;
	hr = GetChildById(L"MainRed", &m_MainRed);
	hasMainRed = hr == S_OK;
	hr = GetChildById(L"MainGreen", &m_MainGreen);
	hasMainGreen = hr == S_OK;
	hr = GetChildById(L"MainBlue", &m_MainBlue);
	hasMainBlue = hr == S_OK;
	hr = GetChildById(L"MainAlpha", &m_MainAlpha);
	hasMainAlpha = hr == S_OK;

	hr = GetChildById(L"ReflectRedValue", &m_ReflectRedValue);
	hasReflectRedValue = hr == S_OK;
	hr = GetChildById(L"ReflectGreenValue", &m_ReflectGreenValue);
	hasReflectGreenValue = hr == S_OK;
	hr = GetChildById(L"ReflectBlueValue", &m_ReflectBlueValue);
	hasReflectBlueValue = hr == S_OK;
	hr = GetChildById(L"ReflectAlphaValue", &m_ReflectAlphaValue);
	hasReflectAlphaValue = hr == S_OK;
	hr = GetChildById(L"ReflectRed", &m_ReflectRed);
	hasReflectRed = hr == S_OK;
	hr = GetChildById(L"ReflectGreen", &m_ReflectGreen);
	hasReflectGreen = hr == S_OK;
	hr = GetChildById(L"ReflectBlue", &m_ReflectBlue);
	hasReflectBlue = hr == S_OK;
	hr = GetChildById(L"ReflectAlpha", &m_ReflectAlpha);
	hasReflectAlpha = hr == S_OK;

	// Selection Display Controls
	hr = GetChildById(L"BkgVisualStat", &m_BkgVisualStat);
	hasBkgVisualStat = hr == S_OK;
	hr = GetChildById(L"IconVisualStat", &m_IconVisualStat);
	hasIconVisualStat = hr == S_OK;
	hr = GetChildById(L"ReflectVisualStat", &m_ReflectVisualStat);
	hasReflectVisualStat = hr == S_OK;
	hr = GetChildById(L"MainTextColorStat", &m_MainTextColorStat);
	hasMainTextColorStat = hr == S_OK;
	hr = GetChildById(L"ReflectTextColorStat", &m_ReflectTextColorStat);
	hasReflectTextColorStat = hr == S_OK;
	hr = GetChildById(L"AnimationStateStat", &m_AnimStateStat);
	hasAnimStateStat = hr == S_OK;
	hr = GetChildById(L"GradientStateStat", &m_GradientStateStat);
	hasGradientStateStat = hr == S_OK;
	hr = GetChildById(L"IconStateStat", &m_IconStateStat);
	hasIconStateStat = hr == S_OK;

	// Visual Controls
	hr = GetChildById(L"VisualName", &m_VisualName);
	hasVisualName = hr == S_OK;
	hr = GetChildById(L"VisualDescription", &m_VisualDescription);
	hasVisualDescription = hr == S_OK;

	// Preview Controls
	hr = GetChildById(L"ContentTab_Background", &m_Background);
	hasBackground = hr == S_OK;
	hr = GetChildById(L"ContentTab_Icon", &m_Icon);
	hasIcon = hr == S_OK;
	hr = GetChildById(L"ContentTab_Reflection", &m_Reflection);
	hasReflection = hr == S_OK;
	hr = GetChildById(L"ContentTab_Animation", &m_Animation);
	hasAnimation = hr == S_OK;
	hr = GetChildById(L"ContentTab_Gradient", &m_Gradient);
	hasGradient = hr == S_OK;
	hr = GetChildById(L"ContentTab_MainText", &m_MainText);
	hasMainText = hr == S_OK;
	hr = GetChildById(L"ContentTab_ReflectionText", &m_ReflectText);
	hasReflectText = hr == S_OK;

	hr = GetChildById(L"SelectTab", &m_SelectTab);
	hasSelectTab = hr == S_OK;
	hr = GetChildById(L"ApplyTabChange", &m_ApplyTabChange);
	hasApplyTabChange = hr == S_OK;
	hr = GetChildById(L"SaveChanges", &m_SaveChanges);
	hasSaveChanges = hr == S_OK;
	hr = GetChildById(L"ApplyToMenu", &m_ApplyToMenu);
	hasApplyToMenu = hr == S_OK;
	hr = GetChildById(L"ApplyToDash", &m_ApplyToDash);
	hasApplyToDash = hr == S_OK;
	hr = GetChildById(L"Randomize", &m_Randomize);
	hasRandomize = hr == S_OK;
	hr = GetChildById(L"RandomStatus", &m_RandomStatus);
	hasRandomStatus = hr == S_OK;

}

void CScnTabConfig::UpdateTabPreview(TabDataPackage tabData)
{
	int nVisual, nIndex;
	string strPath;

	m_Lock.Lock();
	if(hasBackground)
	{
		nVisual = tabData.ContentBkgVisual;
		nIndex = tabData.ContentBkgIndex;

		strPath = TabManager::getInstance().getAssetData(nVisual, nIndex, TM_ASSET_BKG).ImagePath;
		
		m_Background.SetImagePath(strtowstr(strPath).c_str());
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
	if(hasGradient)
		m_Gradient.SetShow(tabData.ContentGradientState ? TRUE : FALSE);

	if(hasAnimation)
		m_Animation.SetShow(tabData.ContentAnimState ? TRUE : FALSE);

	if(hasIcon)
		m_Icon.SetShow(tabData.ContentIconState ? TRUE : FALSE);

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
		tabData.ContentMainTextColor = D3DCOLOR_ARGB(a,r,g,b);
		propvalTextColor.SetColorVal(tabData.ContentMainTextColor);
		XuiObjectSetProperty( m_MainText.m_hObj, dwIdTextColor, 0, &propvalTextColor );
		m_MainText.SetOpacity(opVal);
	}
	if(hasReflectText)
	{
		DWORD dwIdTextColor = 0;
		XUIElementPropVal propvalTextColor;
		XuiObjectGetPropertyId( m_ReflectText.m_hObj, L"TextColor", &dwIdTextColor );
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE a = (BYTE) (txtColor >> 24);
		BYTE r = (BYTE) ((txtColor >> 16) & 0xFF);
		BYTE g = (BYTE) ((txtColor >> 8 ) & 0xFF);
		BYTE b = (BYTE) (txtColor & 0xFF);
		float opVal = a / 255.0f;
		a = 0xFF;
		tabData.ContentReflectTextColor = D3DCOLOR_ARGB(a,r,g,b);
		propvalTextColor.SetColorVal(tabData.ContentReflectTextColor);
		XuiObjectSetProperty( m_ReflectText.m_hObj, dwIdTextColor, 0, &propvalTextColor );
		m_ReflectText.SetOpacity(opVal);
	}

	this->SetText(strtowstr(tabData.ContentDisplayText).c_str());
	
	m_Lock.Unlock();
}


	
void CScnTabConfig::SetCurrentItems( void )
{
	//TabDataPackage tabData = TabManager::getInstance().requestTabDataPackage(focusParent, focusChild, TM_MENU_PREVIEW);
	TabDataPackage tabData = TabConfigHub::getInstance().getPreviewData();
	string temp;

	if(hasBkgList)
	{
		m_BkgList.SetItemCheck(tabData.ContentBkgVisual, true);
	}
	if(hasIconList)
	{
		m_IconList.SetItemCheck(tabData.ContentIconVisual, true);
	}
	if(hasReflectionList)
	{
		m_ReflectionList.SetItemCheck(tabData.ContentReflectVisual, true);
	}

	if(hasMainRedValue)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE r = (BYTE)((txtColor >> 16) & 0xFF);
		string strValue = sprintfaA("%d", r);
		m_MainRedValue.SetText(strtowstr(strValue).c_str());
	}
	if(hasMainGreenValue)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE g = (BYTE)((txtColor >> 8 ) & 0xFF);
		string strValue = sprintfaA("%d", g);
		m_MainGreenValue.SetText(strtowstr(strValue).c_str());
	}
	if(hasMainBlueValue)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE b = (BYTE)(txtColor & 0xFF);
		string strValue = sprintfaA("%d", b);
		m_MainBlueValue.SetText(strtowstr(strValue).c_str());
	}
	if(hasMainAlphaValue)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE a = (BYTE)(txtColor >> 24);
		string strValue = sprintfaA("%d", a);
		m_MainAlphaValue.SetText(strtowstr(strValue).c_str());
	}

	if(hasMainRed)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE r = (BYTE)((txtColor >> 16) & 0xFF);
		m_MainRed.SetValue((int)r);
	}
	if(hasMainGreen)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE g = (BYTE)((txtColor >> 8 ) & 0xFF);
		m_MainGreen.SetValue((int)g);
	}
	if(hasMainBlue)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE b =(BYTE) (txtColor & 0xFF);
		m_MainBlue.SetValue((int)b);
	}
	if(hasMainAlpha)
	{
		DWORD txtColor = tabData.ContentMainTextColor;
		BYTE a = (BYTE)(txtColor >> 24);
		m_MainAlpha.SetValue((int)a);
	}

	if(hasReflectRedValue)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE r = (BYTE)((txtColor >> 16) & 0xFF);
		string strValue = sprintfaA("%d", r);
		m_ReflectRedValue.SetText(strtowstr(strValue).c_str());
	}
	if(hasReflectGreenValue)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE g = (BYTE)((txtColor >> 8 ) & 0xFF);
		string strValue = sprintfaA("%d", g);
		m_ReflectGreenValue.SetText(strtowstr(strValue).c_str());
	}
	if(hasReflectBlueValue)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE b = (BYTE)(txtColor & 0xFF);
		string strValue = sprintfaA("%d", b);
		m_ReflectBlueValue.SetText(strtowstr(strValue).c_str());
	}
	if(hasReflectAlphaValue)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE a = (BYTE)(txtColor >> 24);
		string strValue = sprintfaA("%d", a);
		m_ReflectAlphaValue.SetText(strtowstr(strValue).c_str());
	}

	if(hasReflectRed)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE r = (BYTE)((txtColor >> 16) & 0xFF);
		m_ReflectRed.SetValue((int)r);
	}
	if(hasReflectGreen)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE g = (BYTE)((txtColor >> 8 ) & 0xFF);
		m_ReflectGreen.SetValue((int)g);
	}
	if(hasReflectBlue)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE b = (BYTE)(txtColor & 0xFF);
		m_ReflectBlue.SetValue((int)b);
	}
	if(hasReflectAlpha)
	{
		DWORD txtColor = tabData.ContentReflectTextColor;
		BYTE a = (BYTE)(txtColor >> 24);
		m_ReflectAlpha.SetValue((int)a);
	}
}