#pragma once
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "DebugCommand.h"

class ScreenshotCommand:public DebugCommand
{
private :
	static bool m_TakeScreenshot;
	
public :
	static bool getScreenshotRequest();
	
    static void TakeScreenshot();
	
	ScreenshotCommand();
	
	void Perform(string parameters);
	
	
};