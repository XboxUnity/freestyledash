#include "stdafx.h"
#include "ScnPluginTrainers.h"

HRESULT CScnPluginTrainers::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("ScnPluginTrainers","OnInit");

	HRESULT hr = GetChildById( L"Trainers", &m_Trainers );
	isNested = hr == S_OK;

	DebugMsg("ScnPluginTrainers","Scn is nested %d", isNested);
	if(isNested)
	{
		m_Trainers.GetChildById(L"TrainerTitle",  &m_TrainerTitle );
		m_Trainers.GetChildById(L"TrainerDescription", &m_TrainerDescrip );
		m_Trainers.GetChildById(L"TrainerAuthor", &m_TrainerAuthor );
		m_Trainers.GetChildById(L"TrainerList", &m_TrainerList );
	}
	else
	{
		GetChildById(L"TrainerTitle",  &m_TrainerTitle );
		GetChildById(L"TrainerDescription", &m_TrainerDescrip );
		GetChildById(L"TrainerAuthor", &m_TrainerAuthor );
		GetChildById(L"TrainerList", &m_TrainerList );
	}

	return S_OK;
}