#include "stdafx.h"
#include "../GPD/GPD.h"
#include "GameGPD.h"

HRESULT GameGPD::GetAchievementImage(__int64 AchievementImageID, BYTE ** ImageData, DWORD * ImageSize)
{

	HRESULT retVal = NULL;
	BYTE * gpdData;
	int gpdSize = 0;

	retVal = GetSection(GPD_SECTION_IMAGES, AchievementImageID, &gpdData, &gpdSize);
	if(retVal != 1)
		return S_FALSE;

	*ImageData = gpdData;
	*ImageSize = (DWORD)gpdSize;

	return S_OK;
}

HRESULT GameGPD::GetAchievementEntry(__int64 AchievementID, PXACHIEVEMENT_DETAILS * AchievementDetails)
{
	HRESULT retVal = NULL;
	BYTE * gpdData = NULL;
	int gpdSize = 0;
	
	GPDACHVHeader * achvHeader = (GPDACHVHeader *)malloc(sizeof(GPDACHVHeader));
	*AchievementDetails = (PXACHIEVEMENT_DETAILS)malloc(XACHIEVEMENT_SIZE_FULL);

	retVal = GetSection(GPD_SECTION_ACHIEVEMENT, AchievementID, &gpdData, &gpdSize);
	if(retVal != 1)
		return S_FALSE;

	memset(achvHeader, 0, sizeof(GPDACHVHeader));
	memcpy(achvHeader, gpdData, sizeof(GPDACHVHeader));

	(*AchievementDetails)->dwCred = achvHeader->dwGamerCred;
	(*AchievementDetails)->dwFlags = achvHeader->dwFlags;
	(*AchievementDetails)->dwId= achvHeader->dwAchievementID;
	(*AchievementDetails)->dwImageId = achvHeader->dwImageID;
	(*AchievementDetails)->ftAchieved = achvHeader->ftUnlockTime;

	WCHAR * szRawData = (WCHAR*)(gpdData + achvHeader->dwStructSize);
	int nRawStrLen = (gpdSize - achvHeader->dwStructSize) / 2;

	int nNullCount = 0;
	long entryAddress = (long)gpdData;
	long prevOffset = 0;
	long nextOffset = achvHeader->dwStructSize;
	
	for(int nOffset = 0; nOffset < nRawStrLen; nOffset++){
		if(szRawData[nOffset] == 0){
			// Found Null, process
			prevOffset = nextOffset;
			nextOffset = achvHeader->dwStructSize + ((nOffset + 1) * 2);
			DWORD dwChunkSize = nextOffset - prevOffset;
			switch (nNullCount){
				case GPD_STRING_LABEL:
					(*AchievementDetails)->pwszLabel = (LPWSTR)malloc(XACHIEVEMENT_MAX_LABEL_LENGTH);
					memset((*AchievementDetails)->pwszLabel, 0, XACHIEVEMENT_MAX_LABEL_LENGTH);
					memcpy((*AchievementDetails)->pwszLabel, (void*)(entryAddress + prevOffset), dwChunkSize);
					break;
				case GPD_STRING_DESC:
					(*AchievementDetails)->pwszDescription = (LPWSTR)malloc(XACHIEVEMENT_MAX_DESC_LENGTH);
					memset((*AchievementDetails)->pwszDescription, 0, XACHIEVEMENT_MAX_LABEL_LENGTH);
					memcpy((*AchievementDetails)->pwszDescription, (void*)(entryAddress + prevOffset), dwChunkSize);
					break;
				case GPD_STRING_UNACH:
					(*AchievementDetails)->pwszUnachieved = (LPWSTR)malloc(XACHIEVEMENT_MAX_UNACH_LENGTH);
					memset((*AchievementDetails)->pwszUnachieved, 0, XACHIEVEMENT_MAX_LABEL_LENGTH);
					memcpy((*AchievementDetails)->pwszUnachieved, (void*)(entryAddress + prevOffset), dwChunkSize);					
					break;
			};
			nNullCount++;
		}
	}
	
	if(gpdData != NULL)
		delete [] gpdData;


	return S_OK;
}