#pragma once

#include "../../Generic/tools.h"
#include "../../Debug/Debug.h"
#include "../../Settings/Settings.h"
#include "../Skin/SkinManager.h"
#include "../Skin/SkinXMLReader/SkinXMLReader.h"
#include "../../../Application/FreestyleApp.h"

class VideoInfo {
public:
	string Name;
	string Path;
	string Type;
	string IconPath;
	XMEDIA_AUDIO_DESCRIPTOR audioInfo;
	XMEDIA_VIDEO_DESCRIPTOR videoInfo;
};


class VideoManager : SkinXMLReader
{
public:

	static VideoManager& getInstance()
	{
		static VideoManager singleton;
		return singleton;
	}
	
	int getVideoNum()
	{
		return VideoNo;
	}

	string getVideoPath(int Num)
	{
		return VideoList[Num].Path;
	}

	string getVideoName(int Num)
	{
		return VideoList[Num].Name;
	}  

	string getVideoType(int Num)
	{
		return VideoList[Num].Type;
	}  

	string getIconPath(int Num)
	{
		return VideoList[Num].IconPath;
	}

	XMEDIA_AUDIO_DESCRIPTOR getAudioInfo(int Num)
	{
		return VideoList[Num].audioInfo;
	}

	XMEDIA_VIDEO_DESCRIPTOR getVideoInfo(int Num)
	{
		return VideoList[Num].videoInfo;
	}
	

private:
	
	string m_BasePath, szBaseSkinPath;
	map<int , VideoInfo> VideoList;
	int VideoNo;
	IXMedia2XmvPlayer* xmvPlayer;
	XMEDIA_XMV_CREATE_PARAMETERS parameters;
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
	vector<string> m_Paths; //Image Paths


	void DefaultSettings();
	void getVideoList();

	void GetDetailedVideoInfo(VideoInfo Video)
	{
		string path = Video.Path;
		parameters.createFromFile.szFileName = path.c_str();
		XMedia2CreateXmvPlayer(CFreestyleApp::getInstance().m_pd3dDevice , pXAudio2, &parameters, &xmvPlayer);
		xmvPlayer->GetAudioDescriptor(&Video.audioInfo);
		xmvPlayer->GetVideoDescriptor(&Video.videoInfo);

		VideoList[VideoNo] = Video;
		VideoNo++;
	}

	void InitializeTempVideo()
	{
		//Set Up Default Parameters
		pXAudio2 = NULL;
		pMasterVoice = NULL;
		xmvPlayer = NULL;
		XAudio2Create( &pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ;
		pXAudio2->CreateMasteringVoice( &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL ) ;
		parameters.createType = XMEDIA_CREATE_FROM_FILE;
		parameters.dwAudioStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
		parameters.dwVideoStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
	}

	void Cleanup()
	{
		xmvPlayer->Release();
		pXAudio2->Release();
		pXAudio2 = 0;
		xmvPlayer = 0;
	}

	VideoManager()
	{
		DefaultSettings();
	}

	~VideoManager() 
	{
	}
	VideoManager(const VideoManager&);                 // Prevent copy-construction
	VideoManager& operator=(const VideoManager&);      // Prevent assignment

};