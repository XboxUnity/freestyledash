#include "stdafx.h"

#include "CanvasPresenter.h"
#include "../../../../Tools/Threads/ThreadLock.h"
#include "../../../../Tools/Generic/xboxtools.h"
#include "../../../../Tools/Managers/Theme/CanvasManager/CanvasManager.h"

using namespace std;

CanvasPresenter::~CanvasPresenter()
{

}
HRESULT CanvasPresenter::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{

	return S_OK; 
}