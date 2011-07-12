#pragma once

#include "../../../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../../../Abstracts/HideableScene/HideableScene.h"
#include "../../../../Tools/Managers/Theme/CanvasManager/CanvasManager.h"

class CanvasPresenter: public ConfigurableScene
{
protected :

public :
	
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
	XUI_END_MSG_MAP()

	XUI_IMPLEMENT_CLASS(CanvasPresenter,L"CanvasPresenter",XUI_CLASS_SCENE);

	HRESULT CanvasPresenter::OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	~CanvasPresenter();
	
};
