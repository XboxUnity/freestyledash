#include "stdafx.h"

#include "TrainerList.h"

CTrainerList::CTrainerList()
{

}

CTrainerList::~CTrainerList()
{ 

}

HRESULT CTrainerList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	return S_OK;
}

HRESULT CTrainerList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	return S_OK;
}

HRESULT CTrainerList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
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

HRESULT CTrainerList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	return S_OK;
}

HRESULT CTrainerList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = cItems;
	m_bListReady = true;
	bHandled = TRUE;
	return S_OK;
}
