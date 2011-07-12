#include "stdafx.h"

#include "../../../Application/FreestyleApp.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "ScreenshotCommand.h"
#include "../../Generic/xboxtools.h"
#include "../../Generic/PNG/lodepng.h"
#include "../../Texture/TextureCache.h"
#include "../../XEX/Xbox360Container.h"

bool ScreenshotCommand::m_TakeScreenshot = false;
bool ScreenshotCommand::getScreenshotRequest()
{
	return m_TakeScreenshot;
}
void ScreenshotCommand::TakeScreenshot()
{
	string path = ScreenShot(ATG::g_pd3dDevice);

	DebugMsg("DebugCommand","DEBUG-DISPLAY:%s",path.c_str());
	m_TakeScreenshot = false;
}
ScreenshotCommand::ScreenshotCommand()
{
	m_CommandName = "Screenshot";
	m_TakeScreenshot = false;
}
void ScreenshotCommand::Perform(string parameters)
{
	if(parameters == "1")
	{

	}
	else
	{
		m_TakeScreenshot=true;
	}

		
}
	
