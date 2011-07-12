#pragma once
#include "../GPD/GPD.h"

class GameGPD : public GPD
{
public:
	GameGPD() {}
	~GameGPD() {}

	HRESULT GetAchievementEntry(__int64 AchievementID, PXACHIEVEMENT_DETAILS * AchievementDetails);
	HRESULT GetAchievementImage(__int64 AchievementImageID, BYTE ** ImageData, DWORD * ImageSize);
private:
};