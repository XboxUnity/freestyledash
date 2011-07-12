#include "stdafx.h"

#include "ScnCanvasPresenter.h"
#include "../../../../Tools/Threads/ThreadLock.h"
#include "../../../../Tools/Generic/xboxtools.h"
#include "../../../../Tools/Managers/Theme/CanvasManager/CanvasManager.h"

using namespace std;

CScnCanvasPresenter::~CScnCanvasPresenter()
{
	if(m_bIsLinked)  
		CanvasManager::getInstance().remove(*this);

}
HRESULT CScnCanvasPresenter::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	HRESULT hr = NULL;
	string strTemp = "";
	
	string strBuffer[MAX_ANIM_ITEMS];	
	bool m_bAnim[MAX_ANIM_ITEMS];

	LoadSettings("ScnCanvasPresenter", *this);

	hr = GetChildById(L"BackgroundA", &m_BackgroundA);
	hasBackgroundA = hr == S_OK;

	hr = GetChildById(L"BackgroundB", &m_BackgroundB);
	hasBackgroundB = hr == S_OK;

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		strTemp = sprintfaA("Animation%d", i);	
		hr = GetChildById(strtowstr(strTemp).c_str(), &m_Animation[i]);
		hasAnimation[i] = hr == S_OK;
	}

	activeBkgAPath = GetSetting( "BKGAPATH", "" );
	activeBkgBPath = GetSetting( "BKGBPATH", "" );

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		
		strTemp = sprintfaA("SHOWANIM%d", i);
		strBuffer[i] = GetSetting(strTemp, "TRUE");

		if(strcmp(strBuffer[i].c_str(), "TRUE") == 0)
			m_bAnim[i] = true;
		else
			m_bAnim[i] = false;
	}
	
	string LinkStatus = GetSetting("MANAGER", "LINKED");

	if(strcmp(LinkStatus.c_str(), "LINKED")==0)
	{
		m_bIsLinked = true;
		CanvasManager::getInstance().add(*this);
	}
	else
		m_bIsLinked = false;


	if(!m_bIsLinked)
	{
		m_lock.Lock();
		if(hasBackgroundA)
			m_BackgroundA.SetImagePath(strtowstr(activeBkgAPath).c_str());
		if(hasBackgroundB)
			m_BackgroundB.SetImagePath(strtowstr(activeBkgBPath).c_str());
		m_lock.Unlock();

		for(int i = 0; i < MAX_ANIM_ITEMS; i++)
		{
			if(hasAnimation[i])
			{
				if(m_bAnim[i])
					m_Animation[i].SetShow(true);
				else
					m_Animation[i].SetShow(false);
			}
		}
	}
	else
	{
		BackgroundElement bkgItemA = CanvasManager::getInstance().getBackgroundA(0, VM_OPTION_LOADED);
		BackgroundElement bkgItemB = CanvasManager::getInstance().getBackgroundB(0, VM_OPTION_LOADED);
		//PrefixElement prefixItem = CanvasManager::getInstance().getPrefix(0, VM_OPTION_LOADED);
		AnimationElement animItem = CanvasManager::getInstance().getAnimation(VM_OPTION_LOADED);
		
		DebugMsg("CanvasPresenter", "Background B Path: %s", bkgItemB.strBackgroundPath.c_str());

		CanvasManager::getInstance().setBackgroundA(bkgItemA, VM_OPTION_ACTIVE);
		CanvasManager::getInstance().setBackgroundB(bkgItemB, VM_OPTION_ACTIVE);
		//CanvasManager::getInstance().setPrefix(prefixItem, VM_OPTION_ACTIVE);
		CanvasManager::getInstance().setAnimation(animItem, VM_OPTION_ACTIVE);

	/*	string currentPrefix = wstrtostr(XuiElementGetVisualPrefix(m_hObj));
		string loadedPrefix = prefixItem.strPrefixID;

		if(strcmp(currentPrefix.c_str(), loadedPrefix.c_str()) != 0)
		{
			XuiElementSetVisualPrefix(m_hObj, strtowstr(loadedPrefix).c_str());
			XuiElementSkinChanged(m_hObj);
			XuiElementSetFocus(m_hObj);
		}*/
	}

	return S_OK; 
}

void CScnCanvasPresenter::UpdateBackgroundA()
{
	BackgroundElement curBackground = CanvasManager::getInstance().getBackgroundA(0, VM_OPTION_ACTIVE);
	
	if(hasBackgroundA)
	{
		DebugMsg("CanvasPresenter", "Has BackgroundA Present: %s", curBackground.strBackgroundName.c_str());
		m_lock.Lock();
		m_BackgroundA.SetImagePath(L"");
		m_BackgroundA.SetImagePath(strtowstr(curBackground.strBackgroundPath).c_str());
		m_lock.Unlock();
	}
}

void CScnCanvasPresenter::UpdateBackgroundB()
{
	BackgroundElement curBackground = CanvasManager::getInstance().getBackgroundB(0, VM_OPTION_ACTIVE);
	
	if(hasBackgroundB)
	{
		DebugMsg("CanvasPresenter", "Has BackgroundB Present: %s", curBackground.strBackgroundName.c_str());
		m_lock.Lock();
		m_BackgroundB.SetImagePath(L"");
		m_BackgroundB.SetImagePath(strtowstr(curBackground.strBackgroundPath).c_str());
		m_lock.Unlock();
	}
}

void CScnCanvasPresenter::UpdatePrefix()
{
	PrefixElement curPrefix = CanvasManager::getInstance().getPrefix(0, VM_OPTION_ACTIVE);

	// Update prefix here, once possible
}

void CScnCanvasPresenter::UpdateAnimation()
{
	AnimationElement curAnim = CanvasManager::getInstance().getAnimation(VM_OPTION_ACTIVE);

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		if(hasAnimation[i])
			m_Animation[i].SetShow(curAnim.animItem[i]);
	}
}