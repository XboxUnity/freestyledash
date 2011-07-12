#include "stdafx.h"

#include "TrainerOptionList.h"

CTrainerOptionList::CTrainerOptionList()
{

}

CTrainerOptionList::~CTrainerOptionList()
{ 

}

HRESULT CTrainerOptionList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	return S_OK;
}

HRESULT CTrainerOptionList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	return S_OK;
}

HRESULT CTrainerOptionList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 1 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		bHandled = TRUE;
	}
	if( ( 2 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		bHandled = TRUE;
	}

    return S_OK;
}

HRESULT CTrainerOptionList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	return S_OK;
}

HRESULT CTrainerOptionList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = cItems;
	m_bListReady = true;
	bHandled = TRUE;
	return S_OK;
}
