#pragma once

#include "../../../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../../../Abstracts/HideableScene/HideableScene.h"
#include "../../../../Tools/Managers/Theme/CanvasManager/CanvasManager.h"
#include "../../../../Tools/Threads/ThreadLock.h"

class CScnCanvasPresenter:public ConfigurableScene, public iThemeCanvasObserver
{
protected :
	CXuiImageElement m_BackgroundA;
	CXuiImageElement m_BackgroundB;
	CXuiControl m_Animation[MAX_ANIM_ITEMS];
	
	bool m_bIsLinked;
	
	string activeBkgAPath;
	string activeBkgBPath;

	ThreadLock m_lock;

	bool hasBackgroundA;
	bool hasBackgroundB;
	bool hasAnimation[MAX_ANIM_ITEMS];

public :
	
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
	XUI_END_MSG_MAP()

	XUI_IMPLEMENT_CLASS(CScnCanvasPresenter,L"ScnCanvasPresenter",XUI_CLASS_SCENE);

	HRESULT CScnCanvasPresenter::OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	~CScnCanvasPresenter();
	
	void UpdateBackgroundA();
	void UpdateBackgroundB();
	void UpdatePrefix();
	void UpdateAnimation();

	// iVisualObserver Implementation
	virtual void handleBackgroundAChange()	{ UpdateBackgroundA(); }
	virtual void handleBackgroundBChange()	{ UpdateBackgroundB(); }
	virtual void handleAnimationChange()	{ UpdateAnimation(); }
	virtual void handlePrefixChange()		{ UpdateAnimation(); }
};
