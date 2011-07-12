#pragma once

#include "../../../../Tools/Threads/ThreadLock.h"
#include "../../../../Tools/Generic/xboxtools.h"
#include "../../../../Tools/Managers/Theme/CanvasManager/CanvasManager.h"

#define MAX_CONTROL_INDEX 5

enum ControlIndex {
	CON_LIST,
	CON_PREVIEWIMAGE,
	CON_FULLSCREENIMAGE,
	CON_CURRENTTEXT,
	CON_NEWTEXT
};

class iCanvasConfigObserver{
public :
	virtual void handleBackgroundAChange() = 0;
	virtual void handleBackgroundBChange() = 0;
	virtual void handleAnimationChange() = 0;
};

class CScnCanvasConfig : public CXuiTabSceneImpl, public iCanvasConfigObserver
{
protected :

	ThreadLock m_lock;

	CXuiList m_BkgListA, m_BkgListB;

	CXuiImageElement m_BkgPreviewImageA, m_BkgFullscreenA;
	CXuiImageElement m_BkgPreviewImageB, m_BkgFullscreenB;

	CXuiTextElement m_CurBkgNameA, m_CurBkgNameB;
	CXuiTextElement m_NewBkgNameA, m_NewBkgNameB;

	CXuiControl m_Animation[MAX_ANIM_ITEMS];

	CXuiControl m_ApplyChanges;
	
	CXuiCheckbox m_EnableSlideshow;
	CXuiCheckbox m_EnableRandom;
	CXuiCheckbox m_EnableAnimation[MAX_ANIM_ITEMS];

	bool hasBackgroundA[sizeof(ControlIndex)];
	bool hasBackgroundB[sizeof(ControlIndex)];
	bool hasAnimation[MAX_ANIM_ITEMS];
	
	bool hasApplyButton;
	bool hasEnableSS, hasEnableRandom;
	bool hasEnableAnimation[MAX_ANIM_ITEMS];

	HRESULT InitializeChildren( void );
	void FillBackgroundListA( void );
	void FillBackgroundListB( void );
	void InitialSettingsRead( void );
	
	void SetCurrentBkgA( void );
	void SetCurrentBkgB( void );

public :
	
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED ( OnNotifySelchanged )
	XUI_END_MSG_MAP()

	~CScnCanvasConfig();
	XUI_IMPLEMENT_CLASS(CScnCanvasConfig, L"ScnCanvasConfig", XUI_CLASS_TABSCENE);

	HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifySelchanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled );

	void UpdateBackgroundA();
	void UpdateBackgroundB();
	void UpdateAnimation();

	virtual void handleBackgroundAChange()
	{
		UpdateBackgroundA();
	}

	virtual void handleBackgroundBChange()
	{
		UpdateBackgroundB();
	}
	virtual void handleAnimationChange()
	{
		UpdateAnimation();
	}
};

class CConfigManager
{
public:
	static CConfigManager& getInstance()
	{
		static CConfigManager singleton;
		return singleton;
	}

	void add(iCanvasConfigObserver& ref);
	void remove(iCanvasConfigObserver& ref);

	BackgroundElement getBackgroundA();
	BackgroundElement getBackgroundB();
	AnimationElement getAnimation();

	void setBackgroundA(BackgroundElement bkgItem);
	void setBackgroundB(BackgroundElement bkgItem);
	void setAnimation(AnimationElement animItem);

private:
	std::map<iCanvasConfigObserver* const,iCanvasConfigObserver* const> _observers;
	typedef std::map<iCanvasConfigObserver* const, iCanvasConfigObserver* const> item;
	
	BackgroundElement selBkgA;
	BackgroundElement selBkgB;
	AnimationElement selAnim;

	void _notifyBackgroundAChange();
	void _notifyBackgroundBChange();
	void _notifyAnimationChange();

	CConfigManager(){}							        // Private constructor
	~CConfigManager(){}
	
	CConfigManager(const CConfigManager&);		            // Prevent copy-construction
	CConfigManager& operator=(const CConfigManager&);		    // Prevent assignment
};