#include "stdafx.h"

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Generic/xboxtools.h"
#include "../ConfigurableScene/ConfigurableScene.h"
#include "HideableScene.h"

using namespace std;

HideableScene::HideableScene()
{	
	CurrentlyShown = this->IsShown();
}

void HideableScene::Hide()
{
	CurrentlyShown = this->IsShown();
	if(CurrentlyShown == TRUE)
	{
	switch(m_HideMode)
	{
		case 1:
			this->SetShow(false);
			break;
		case 2 :
			//DebugMsg("Hiding Now");
			int FrameStart;
			int FrameEnd;
			this->FindNamedFrame(strtowstr(StartHideFrameName).c_str(),&FrameStart);
			this->FindNamedFrame(strtowstr(EndHideFrameName).c_str(),&FrameEnd);

			this->PlayTimeline(FrameStart,FrameStart,FrameEnd,false,false);
			break;
	}
	}
}
void HideableScene::Show()
{
	//DebugMsg("Show:  WE made it into the function");
	//CurrentlyShown = this->IsShown();
	
	// This needs to be fixed- will revist
	CurrentlyShown = FALSE;
	if(CurrentlyShown == FALSE)
	{
	//DebugMsg("We passed CurrentlyShown Test");
	switch(m_HideMode)
		{
		case 1:
			this->SetShow(true);
		
			break;
		case 2 :
			DebugMsg("HideableScene","Showing Now");
			int FrameStart;
			int FrameEnd;
			this->FindNamedFrame(strtowstr(StartShowFrameName).c_str(),&FrameStart);
			this->FindNamedFrame(strtowstr(EndShowFrameName).c_str(),&FrameEnd);
	
			this->PlayTimeline(FrameStart,FrameStart,FrameEnd,false,false);
			break;
		}
	}
}
void HideableScene::SettingsLoaded()
{
	string hideMode = GetSetting( "HIDEMODE","OFF");
	if(hideMode.compare("OFF")==0)
	{
		m_HideMode= 0;
	
	}
	else
	{
		if(hideMode.compare("HIDE")==0)
		{
			m_HideMode = 1;
			
		}
		else
		{
			if(hideMode.compare("TRANS")==0)
			{
			
				m_HideMode = 2;
			}
			else
			{
				m_HideMode = 0;
			}
		}
	}

	StartShowFrameName = GetSetting( "StartShowFrameName","TransTo");
	EndShowFrameName = GetSetting( "EndShowFrameName","EndTransTo");
	StartHideFrameName = GetSetting( "StartHideFrameName","TransFrom");
	EndHideFrameName = GetSetting( "EndHideFrameName","EndTransFrom");	
	
}