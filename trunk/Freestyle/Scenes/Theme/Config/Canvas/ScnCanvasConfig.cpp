#include "stdafx.h"
#include "ScnCanvasConfig.h"

#include "../../../../Tools/Settings/Settings.h"
#include "../../../../Tools/Managers/Skin/SkinManager.h"
#include "../../../../Tools/Threads/ThreadLock.h"
#include "../../../../Tools/Generic/xboxtools.h"

using namespace std;

CScnCanvasConfig::~CScnCanvasConfig()
{
	CConfigManager::getInstance().remove(*this);
}

HRESULT CScnCanvasConfig::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{

	CConfigManager::getInstance().add(*this);
	DebugMsg("CanvasConfig", "Canvas Config Initialized");

	InitializeChildren();
	
	FillBackgroundListA();
	FillBackgroundListB();

	InitialSettingsRead();

	SetCurrentBkgA();
	SetCurrentBkgB();

    return S_OK;
}

HRESULT CScnCanvasConfig::OnNotifySelchanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled )
{
	if(hObjSource == m_BkgListA)
	{
		int nSel = pNotifySel->iItem;
		string strPreviewPath = CanvasManager::getInstance().getBackgroundA(nSel, VM_OPTION_INDEX).strPreviewPath;
		//string strPreviewPath = WallpaperManager::getInstance().getBackground(nSel).PreviewPathName;
		//m_lock.Lock();
		//m_BkgPreviewImageA.SetImagePath(L"");
		m_BkgPreviewImageA.SetImagePath(strtowstr(strPreviewPath).c_str());
		//m_lock.Unlock();
	}
	if(hObjSource == m_BkgListB)
	{
		int nSel = pNotifySel->iItem;
		string strPreviewPath = CanvasManager::getInstance().getBackgroundB(nSel, VM_OPTION_INDEX).strPreviewPath;
		//string strPreviewPath = WallpaperManager::getInstance().getStage(nSel).PreviewPathName;
		//m_lock.Lock();
		//m_BkgPreviewImageB.SetImagePath(L"");
		m_BkgPreviewImageB.SetImagePath(strtowstr(strPreviewPath).c_str());
		//m_lock.Unlock();
	}

	return S_OK;
}

HRESULT CScnCanvasConfig::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	
	if(hObjPressed == m_ApplyChanges)
	{
		BackgroundElement bkgItemA;
		BackgroundElement bkgItemB;
		AnimationElement animItem;

		bkgItemA = CConfigManager::getInstance().getBackgroundA();
		bkgItemB = CConfigManager::getInstance().getBackgroundB();
		animItem = CConfigManager::getInstance().getAnimation();

		CanvasManager::getInstance().setBackgroundA(bkgItemA, VM_OPTION_LOADED);
		CanvasManager::getInstance().setBackgroundB(bkgItemB, VM_OPTION_LOADED);

		CanvasManager::getInstance().setBackgroundA(bkgItemA, VM_OPTION_ACTIVE);
		CanvasManager::getInstance().setBackgroundB(bkgItemB, VM_OPTION_ACTIVE);

		CanvasManager::getInstance().setAnimation(animItem, VM_OPTION_LOADED);
		CanvasManager::getInstance().setAnimation(animItem, VM_OPTION_ACTIVE);

		CanvasManager::getInstance().SaveSettingsToFile();
	}

	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		if(hObjPressed == m_EnableAnimation[i])
		{
			AnimationElement anim = CConfigManager::getInstance().getAnimation();
			//AnimationStruct anim = WallpaperManager::getInstance().getAnimation();

			if(m_EnableAnimation[i].IsChecked() == 1)
				anim.animItem[i] = true;
			else
				anim.animItem[i] = false;


				CConfigManager::getInstance().setAnimation(anim);
			//CanvasManager::getInstance().setAnimation(anim, VM_OPTION_ACTIVE);
			//WallpaperManager::getInstance().setAnimation(anim);
		}
	}

	if(hObjPressed == m_EnableRandom)
	{

		int nMaxBkgA = CanvasManager::getInstance().getBackgroundACount();
		int nMaxBkgB = CanvasManager::getInstance().getBackgroundBCount();

		int nBkgA = rand() % nMaxBkgA;
		//DebugMsg("CanvasConfig", "Random BackgroundA:  %d", nBkgA);
		int nBkgB = rand() % nMaxBkgB;
		//DebugMsg("CanvasConfig", "Random BackgroundB:  %d", nBkgB);

		BackgroundElement bkgItem = CanvasManager::getInstance().getBackgroundA(nBkgA, VM_OPTION_INDEX);
		CConfigManager::getInstance().setBackgroundA(bkgItem);
		
		bkgItem = CanvasManager::getInstance().getBackgroundB(nBkgB, VM_OPTION_INDEX);
		CConfigManager::getInstance().setBackgroundB(bkgItem);
	
	}

	if(hObjPressed == m_BkgListA)
	{
		int nCurSel = m_BkgListA.GetCurSel();
		for(int i = 0; i < m_BkgListA.GetItemCount(); i++)
		{
			m_BkgListA.SetItemCheck(i, false);
		}

		m_BkgListA.SetItemCheck(nCurSel, true);
		BackgroundElement bkgTest = CanvasManager::getInstance().getBackgroundA(nCurSel, VM_OPTION_INDEX);
		//BackgroundStruct bkgTest = WallpaperManager::getInstance().getBackground(nCurSel);

		CConfigManager::getInstance().setBackgroundA(bkgTest);

	}
	if(hObjPressed == m_BkgListB)
	{
		int nCurSel = m_BkgListB.GetCurSel();
		for(int i = 0; i < m_BkgListB.GetItemCount(); i++)
		{
			m_BkgListB.SetItemCheck(i, false);
		}

		m_BkgListB.SetItemCheck(nCurSel, true);
		
		BackgroundElement bkgTest = CanvasManager::getInstance().getBackgroundB(nCurSel, VM_OPTION_INDEX);
		//BackgroundElement bkgTest = WallpaperManager::getInstance().getStage(nCurSel);
		
		CConfigManager::getInstance().setBackgroundB(bkgTest);
	}

	return S_OK;   
} 

void CScnCanvasConfig::InitialSettingsRead( void )
{
	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		if(hasEnableAnimation[i])
		{
			m_EnableAnimation[i].SetCheck(CanvasManager::getInstance().getAnimation(VM_OPTION_LOADED).animItem[i]);
			AnimationElement anim = CanvasManager::getInstance().getAnimation(VM_OPTION_LOADED);

			CConfigManager::getInstance().setAnimation(anim);
		}
		
		if(hasAnimation[i])
		{
			m_Animation[i].SetShow(CConfigManager::getInstance().getAnimation().animItem[i]);
		}
	}
}


void CScnCanvasConfig::FillBackgroundListA( void )
{
	if(!hasBackgroundA[CON_LIST])
		return;
	
	int nItemCount = CanvasManager::getInstance().getBackgroundACount();
	
	if(nItemCount == 0)
		return;

	string strItem = "";

	m_BkgListA.InsertItems(0, nItemCount);
	for(int i = 0; i < nItemCount; i++)
	{
		
		strItem = CanvasManager::getInstance().getBackgroundA(i, VM_OPTION_INDEX).strBackgroundName;

		m_BkgListA.SetText(i, strtowstr(strItem).c_str());
	}
}

void CScnCanvasConfig::FillBackgroundListB( void )
{
	if(!hasBackgroundB[CON_LIST])
		return;
	
	DebugMsg("CanvasConfig", "I need to fill my list");
	int nItemCount = CanvasManager::getInstance().getBackgroundBCount();
	
	if(nItemCount == 0)
		return;

	string strItem = "";

	m_BkgListB.InsertItems(0, nItemCount);
	for(int i = 0; i < nItemCount; i++)
	{
		strItem = CanvasManager::getInstance().getBackgroundB(i, VM_OPTION_INDEX).strBackgroundName;
		//strItem = WallpaperManager::getInstance().getStage(i).StageName;
		m_BkgListB.SetText(i, strtowstr(strItem).c_str());
	}
}

void CScnCanvasConfig::SetCurrentBkgA()
{
	BackgroundElement curBkg = CanvasManager::getInstance().getBackgroundA(0, VM_OPTION_ACTIVE);
	//BackgroundStruct curBkg = WallpaperManager::getInstance().getCurrentBackground();

	if(hasBackgroundA[CON_LIST])
	{
		m_BkgListA.SetItemCheck(curBkg.nIndex, true);
		CConfigManager::getInstance().setBackgroundA(curBkg);
	}
	if(hasBackgroundA[CON_PREVIEWIMAGE])
	{
		int nSel = curBkg.nIndex;
		string strPreviewPath = CanvasManager::getInstance().getBackgroundA(nSel, VM_OPTION_ACTIVE).strPreviewPath;
		//string strPreviewPath = WallpaperManager::getInstance().getBackground(nSel).PreviewPathName;
		m_lock.Lock();
		m_BkgPreviewImageA.SetImagePath(L"");
		m_BkgPreviewImageA.SetImagePath(strtowstr(strPreviewPath).c_str());
		m_lock.Unlock();
	}

	UpdateBackgroundA();
	
}

void CScnCanvasConfig::SetCurrentBkgB()
{
	BackgroundElement curBkg = CanvasManager::getInstance().getBackgroundB(0, VM_OPTION_ACTIVE);
	//StageStruct curStg = WallpaperManager::getInstance().getCurrentStage();

	if(hasBackgroundB[CON_LIST])
	{
		m_BkgListB.SetItemCheck(curBkg.nIndex, true);
		CConfigManager::getInstance().setBackgroundB(curBkg);
	}

	if(hasBackgroundB[CON_PREVIEWIMAGE])
	{
		int nSel = curBkg.nIndex;
		string strPreviewPath = CanvasManager::getInstance().getBackgroundB(nSel, VM_OPTION_ACTIVE).strPreviewPath;
		//string strPreviewPath = WallpaperManager::getInstance().getStage(nSel).PreviewPathName;
		m_lock.Lock();
		m_BkgPreviewImageB.SetImagePath(L"");
		m_BkgPreviewImageB.SetImagePath(strtowstr(strPreviewPath).c_str());
		m_lock.Unlock();
	}

	UpdateBackgroundB();
}

HRESULT CScnCanvasConfig::InitializeChildren()
{
	HRESULT hr = NULL;

	// Background A Controls
	hr = GetChildById(L"BackgroundListA", &m_BkgListA);
	hasBackgroundA[CON_LIST] = hr == S_OK;

	hr = GetChildById(L"BackgroundPreviewA", &m_BkgPreviewImageA);
	hasBackgroundA[CON_PREVIEWIMAGE] = hr == S_OK;

	hr = GetChildById(L"CurrentBkgNameA", &m_CurBkgNameA);
	hasBackgroundA[CON_CURRENTTEXT] = hr == S_OK;

	hr = GetChildById(L"NewBkgNameA", &m_NewBkgNameA);
	hasBackgroundA[CON_NEWTEXT] = hr == S_OK;

	hr = GetChildById(L"BkgFullscreenA", &m_BkgFullscreenA);
	hasBackgroundA[CON_FULLSCREENIMAGE] = hr == S_OK;

	// Background B Controls
	hr = GetChildById(L"BackgroundListB", &m_BkgListB);
	hasBackgroundB[CON_LIST] = hr == S_OK;

	hr = GetChildById(L"BackgroundPreviewB", &m_BkgPreviewImageB);
	hasBackgroundB[CON_PREVIEWIMAGE] = hr == S_OK;

	hr = GetChildById(L"CurrentBkgNameB", &m_CurBkgNameB);
	hasBackgroundB[CON_CURRENTTEXT] = hr == S_OK;

	hr = GetChildById(L"NewBkgNameB", &m_NewBkgNameB);
	hasBackgroundB[CON_NEWTEXT] = hr == S_OK;

	hr = GetChildById(L"BkgFullscreenB", &m_BkgFullscreenB);
	hasBackgroundB[CON_FULLSCREENIMAGE] = hr == S_OK;

	// Animation Controls	
	string strBuffer = "";
	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		strBuffer = sprintfaA("Animation%d", i);
		hr = GetChildById(strtowstr(strBuffer).c_str(), &m_Animation[i]);
		hasAnimation[i] = hr == S_OK;

	}
	
	// Scene Buttons
	hr = GetChildById(L"ApplyChanges", &m_ApplyChanges);
	hasApplyButton = hr == S_OK;

	// Scene CheckBoxes
	hr = GetChildById(L"EnableSlidshow", &m_EnableSlideshow);
	hasEnableSS = hr == S_OK;

	hr = GetChildById(L"EnableRandom", &m_EnableRandom);
	hasEnableRandom = hr == S_OK;

	strBuffer = "";
	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		strBuffer = sprintfaA("EnableAnimation%d", i);
		hr = GetChildById(strtowstr(strBuffer).c_str(), &m_EnableAnimation[i]);
		hasEnableAnimation[i] = hr == S_OK;


	}

	return S_OK;

}
void CScnCanvasConfig::UpdateBackgroundA()
{
	if(hasBackgroundA[CON_CURRENTTEXT])
		m_CurBkgNameA.SetText(strtowstr(CanvasManager::getInstance().getBackgroundA(0, VM_OPTION_ACTIVE).strBackgroundName).c_str());

	if(hasBackgroundA[CON_NEWTEXT])
		m_NewBkgNameA.SetText(strtowstr(CConfigManager::getInstance().getBackgroundA().strBackgroundName).c_str());

	if(hasBackgroundA[CON_FULLSCREENIMAGE])
	{
		m_lock.Lock();
		m_BkgFullscreenA.SetImagePath(strtowstr(CConfigManager::getInstance().getBackgroundA().strBackgroundPath).c_str());
		m_lock.Unlock();
	}
}
void CScnCanvasConfig::UpdateBackgroundB()
{
	if(hasBackgroundB[CON_CURRENTTEXT])
		m_CurBkgNameB.SetText(strtowstr(CanvasManager::getInstance().getBackgroundB(0, VM_OPTION_ACTIVE).strBackgroundName).c_str());

	if(hasBackgroundB[CON_NEWTEXT])
		m_NewBkgNameB.SetText(strtowstr(CConfigManager::getInstance().getBackgroundB().strBackgroundName).c_str());

	if(hasBackgroundB[CON_FULLSCREENIMAGE])
	{
		m_lock.Lock();
		m_BkgFullscreenB.SetImagePath(strtowstr(CConfigManager::getInstance().getBackgroundB().strBackgroundPath).c_str());
		m_lock.Unlock();
	}
}

void CScnCanvasConfig::UpdateAnimation()
{
	for(int i = 0; i < MAX_ANIM_ITEMS; i++)
	{
		if(hasAnimation[i])
			m_Animation[i].SetShow(CConfigManager::getInstance().getAnimation().animItem[i]);
	}
}


// ************************************************************************************* //
// **  Class:        CConfigManager                                                   ** //
// **  Description:  This class is to act as a global communicator between multiple   ** //
// **                instances of the same class when placed within the XUI           ** //
// **                environment.                                                     ** //
// ************************************************************************************* //
void CConfigManager::add(iCanvasConfigObserver& ref)
{
	_observers.insert(item::value_type(&ref,&ref));
}
void CConfigManager::remove(iCanvasConfigObserver& ref)
{
	_observers.erase(&ref);
}
void CConfigManager::_notifyBackgroundAChange()
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleBackgroundAChange();
    }
}
void CConfigManager::_notifyBackgroundBChange()
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleBackgroundBChange();
    }
}

void CConfigManager::_notifyAnimationChange()
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleAnimationChange();
    }
}

BackgroundElement CConfigManager::getBackgroundA()
{
	return selBkgA;
}

BackgroundElement CConfigManager::getBackgroundB()
{
	return selBkgB;
}

AnimationElement CConfigManager::getAnimation()
{
	return selAnim;
}

void CConfigManager::setBackgroundA(BackgroundElement bkgItem)
{
	selBkgA = bkgItem;
	_notifyBackgroundAChange();
}

void CConfigManager::setBackgroundB(BackgroundElement bkgItem)
{
	selBkgB = bkgItem;
	_notifyBackgroundBChange();
}

void CConfigManager::setAnimation(AnimationElement animItem)
{
	selAnim = animItem;
	_notifyAnimationChange();
}