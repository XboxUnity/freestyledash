
#include "stdafx.h"
#include "VideoManager.h"

void VideoManager::DefaultSettings()
{
	m_BasePath = SETTINGS::getInstance().getVideoPath();

	DebugMsg("VideoManager", "VideoPath : %s", m_BasePath.c_str());

	if (m_BasePath == "")
	{
		m_BasePath = "hdd1:\\Videos\\";
		SETTINGS::getInstance().setVideoPath(m_BasePath);
	}

	RecursiveFolderSearch(&m_Paths, m_BasePath, "wmv");

	szBaseSkinPath = "";

	szBaseSkinPath = "file://" + SkinManager::getInstance().getCurrentScenePath();

	LoadSettings("VideoManager", "VideoManager");
	getVideoList();	
}

void VideoManager::getVideoList()
{
	VideoNo = 0;

	vector<string>::iterator itr;

	for(itr = m_Paths.begin(); itr != m_Paths.end(); itr++)
	{
		vector<string> vidName;

		//Split the path to retrieve the filename
		StringSplit((*itr), "\\", &vidName);

		InitializeTempVideo();
		VideoInfo Video;

		//DebugMsg("VideoManager","Test Video Name %s", vidName.at(vidName.size()-1).c_str());
		string tempname = vidName.at(vidName.size()-1);

		Video.Name = TrimRightStr(tempname, ".wmv");

		Video.Path = (*itr);

		Video.Type = "WMV Video"; //Hardcoded For Testing
		Video.IconPath = szBaseSkinPath + GetSetting("VIDEOICONPATH", "");


		GetDetailedVideoInfo(Video);

		Cleanup();
	}
	DebugMsg("VideoManager","Finished Listing Videos");
}