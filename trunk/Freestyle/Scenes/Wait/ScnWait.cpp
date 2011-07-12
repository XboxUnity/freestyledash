#include "stdafx.h"

#include "../../Tools/Settings/Settings.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../ManagePaths/RemovePathCallBack.h"

#include "ScnWait.h"

HRESULT CScnWait::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	if(pInitData->pvInitData != NULL)
	{
		WAIT_INFO * m_pWait = (WAIT_INFO *)pInitData->pvInitData;

		waitTitle = m_pWait->title;
		waitType = m_pWait->type;
		waitReboot = m_pWait->reboot;
	}

	InitializeChildren();

	m_OperationTitle.SetText(waitTitle);

	//Set Timer based on wait type
	if(waitType == 1)
	{
		//Start the file operation
		FileOperationManager::getInstance().DoWork();
		SetTimer(TM_DELETE, 50);
	} else if (waitType == 2)
	{
		RemovePathCallBack::getInstance().Start();
		SetTimer(TM_TIMEOUT, 50);
	}

    return S_OK;
}


HRESULT CScnWait::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	switch(pTimer->nId) {
		case TM_DELETE:
			if(FileOperationManager::getInstance().isDone()) {
				// File Operation is complete
				KillTimer(TM_DELETE);

				if(waitReboot)
					Restart();
				else
					NavigateBack(XUSER_INDEX_ANY);

				bHandled = TRUE;
			}
			break;
		case TM_TIMEOUT:
			if(RemovePathCallBack::getInstance().getDone()) {
				// File Operation is complete
				KillTimer(TM_DELETE);

				if(waitReboot)
					Restart();
				else
					NavigateBack(XUSER_INDEX_ANY);

				bHandled = TRUE;
			}
			break;
	};

    return S_OK;
}

HRESULT CScnWait::InitializeChildren( void )
{
	HRESULT hr = GetChildById( L"WaitScene", &m_Wait );
	isNested = hr == S_OK;

	if(isNested)
	{
		m_Wait.GetChildById(L"OperationTitle", &m_OperationTitle);
	}
	else
	{
		GetChildById(L"OperationTitle", &m_OperationTitle);
	}

	return S_OK;
}