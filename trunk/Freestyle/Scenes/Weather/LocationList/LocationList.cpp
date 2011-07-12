#pragma once

#include "stdafx.h"
#include "LocationList.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../../Tools/Settings/Settings.h"


HRESULT CWeatherLocationList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	DebugMsg("CWeatherLocationList","CWeatherLocationList Init");
	WeatherSearch::getInstance().add(*this);

	m_ListSize = 0;

	m_bListLoaded = false;
	return S_OK;
}

HRESULT CWeatherLocationList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	// Notify the list of its current size
	pGetItemCountData->cItems = m_ListSize;

	bHandled = true;

	return S_OK;
}

HRESULT CWeatherLocationList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {
	
		if(m_bListLoaded)
		{
			temp = strtowstr(m_Names[pGetSourceTextData->iItem]);
			pGetSourceTextData->szText = temp.c_str();
		}
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CWeatherLocationList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData, BOOL& bHandled)
{
	if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) {

		bHandled = TRUE;
	}
	return S_OK;
}

void CWeatherLocationList::UpdateWeatherSearch(vector<LocationSearch> m_currentLocation)
{
	m_ListSize = m_currentLocation.size();

	m_Names = new string[m_ListSize];
	m_ID = new string[m_ListSize];

	vector<LocationSearch>::iterator itr;
	int count = 0;
	for(itr = m_currentLocation.begin(); itr != m_currentLocation.end(); itr++)
	{
		m_Names[count] = (*itr).strName;
		m_ID[count] = (*itr).strID;

		count++;
	}

	InsertItems( 0, m_ListSize);
	SetCurSel(0);

	m_bListLoaded = true;
}




