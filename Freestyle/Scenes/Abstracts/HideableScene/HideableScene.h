#pragma once

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Generic/xboxtools.h"
#include "../ConfigurableScene/ConfigurableScene.h"

using namespace std;

typedef enum
{
	HIDEMODE_OFF,
	HIDEMODE_IMMEDIATE,
	HIDEMODE_TRANSITION
} HideModeEnum;

class HideableScene:public ConfigurableScene
{
private:
	string StartShowFrameName;
	string EndShowFrameName;
	string StartHideFrameName;
	string EndHideFrameName;
	BOOL CurrentlyShown;

protected:
	int m_HideMode;

public :
	HideableScene();
	void Hide();
	void Show();
	void SettingsLoaded();
};