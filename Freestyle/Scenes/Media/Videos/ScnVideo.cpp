//include <xmedia2.h>
//include <xaudio2.h>
#include <map>
#include <direct.h>
#include <string>
//include <d3dx9math.h>
//include <xnamath.h>

#include "stdafx.h"

#include "ScnVideo.h"
#include "../../../Application/FreeStyleApp.h"
#include "../../../Tools/Managers/Video/VideoManager.h"
#include "../../../Tools/Managers/Skin/SkinManager.h"

using namespace std;

HRESULT CScnVideo::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnVideo","Display Video");
	
	playspeed = 1;
	videoAR = 0;
	isFullscreen = false;
	controlsShown = false;

	InitializeChildren();

	totalVideos = VideoManager::getInstance().getVideoNum();

	//Set Title of first Movie
	FillInfo(0);

	//Hide PopUp
	ShowPopUp(false);

	// Read Settings from skin.xml
	LoadSettings("ScnVideo", *this);

	//Get Image Paths
	playPath= "";
	pausePath= "";
	mutePath= "";
	unMutePath= "";

	if(hasPause)
		playPath = GetSetting("PLAYBUTTONPATH", "");
	
	if(hasPause)
		pausePath = GetSetting("PAUSEBUTTONPATH", "");
	
	if(hasVol)
		mutePath = GetSetting("MUTEBUTTONPATH", "");

	if(hasVol)
		unMutePath = GetSetting("UNMUTEBUTTONPATH", "");

	string szBasePath = "";

	szBasePath = "file://" + SkinManager::getInstance().getCurrentScenePath();

	if(strcmp(playPath.c_str(), "") != 0){
		playPath = szBasePath + playPath;
	}

	if(strcmp(pausePath.c_str(), "") != 0){
		pausePath = szBasePath + pausePath;
	}

	if(strcmp(mutePath.c_str(), "") != 0){
		mutePath = szBasePath + mutePath;
	}

	if(strcmp(unMutePath.c_str(), "") != 0){
		unMutePath = szBasePath + unMutePath;
	}


	ZeroMemory(&parameters, sizeof (parameters));
	
	pXAudio2 = NULL;
	pMasterVoice = NULL;
	xmvPlayer = NULL;
	XAudio2Create( &pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ;
	pXAudio2->CreateMasteringVoice( &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL ) ;

	//Set Up Default Parameters
	parameters.createType = XMEDIA_CREATE_FROM_FILE;
	parameters.dwAudioStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
	parameters.dwVideoStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
	parameters.createFromFile.dwIoBlockSize = (DWORD)512000;
	parameters.createFromFile.dwIoBlockCount = (DWORD)10;
	parameters.createFromFile.dwIoBlockJitter = (DWORD)10;

	return S_OK;
}

HRESULT CScnVideo::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_VideoList )
    {
		FillInfo(pNotifySelChangedData->iItem);

		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CScnVideo::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL &bHandled )
{
	if (hObjSource == m_Pause )
	{
		if(m_FullscreenVideo.IsPaused())
			m_ButtonType.SetText(L"Play");
		else if(playspeed != PlaySpeed)
			m_ButtonType.SetText(L"Play");
		else
			m_ButtonType.SetText(L"Pause");
	}
	if (hObjSource == m_PrevChapter)
	{
		m_ButtonType.SetText(L"Previous Chapter");
	}
	if (hObjSource == m_RW)
	{
		m_ButtonType.SetText(L"Rewind");
	}
	if (hObjSource == m_Stop)
	{
		m_ButtonType.SetText(L"Stop");
	}
	if (hObjSource == m_FF)
	{
		m_ButtonType.SetText(L"Fast Forward");
	}
	if (hObjSource == m_NextChapter)
	{
		m_ButtonType.SetText(L"Next Chapter");
	}
	if (hObjSource == m_AspectRatio)
	{
		if(videoAR == 0)
			m_ButtonType.SetText(L"Aspect Ratio - Full Screen w/ AR");
		else if(videoAR == 1)
			m_ButtonType.SetText(L"Aspect Ratio - Full Screen");
		else if(videoAR == 2)
			m_ButtonType.SetText(L"Aspect Ratio - Normal Size");
	}
	if (hObjSource == m_Subtitles)
	{
		m_ButtonType.SetText(L"Subtitles");
	}
	if (hObjSource == m_Info)
	{
		m_ButtonType.SetText(L"Video Info");
	}
	if (hObjSource == m_Vol)
	{
		if (m_FullscreenVideo.IsMuted())
			m_ButtonType.SetText(L"Unmute");
		else
			m_ButtonType.SetText(L"Mute");
	}
	return S_OK;
}



HRESULT CScnVideo::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_VideoList)
	{	
		int cur = m_VideoList.GetCurSel();

		if (xmvPlayer == NULL)
		{
			string name = VideoManager::getInstance().getVideoName(cur);
			string path = VideoManager::getInstance().getVideoPath(cur);
			parameters.createFromFile.szFileName = path.c_str();
			XMedia2CreateXmvPlayer(CFreestyleApp::m_pd3dDevice ,pXAudio2, &parameters, &xmvPlayer);
			XEnableScreenSaver(false);

			//Start Video
			m_FullscreenVideo.SetShow(true);
			m_FullscreenVideo.SetPlayer(xmvPlayer);
			m_Backdrop.SetShow(true);
			active = cur;
			isFullscreen = true;
			controlsShown = false;
			playspeed = PlaySpeed;

			xmvPlayer->GetVideoScreen(&Screen);

			//Focus Video and Disable List
			m_Select.SetFocus(XUSER_INDEX_ANY);
			m_VideoList.SetEnable(false);

		}
		bHandled = TRUE;

	} else if (hObjPressed == m_Back)
	{
		if(isFullscreen)
		{
			if(controlsShown)
			{
				ShowPopUp(false);
				controlsShown = false;
				m_Select.SetFocus(XUSER_INDEX_ANY);
			} else
			{
				xmvPlayer->Stop(XMEDIA_STOP_IMMEDIATE);
				xmvPlayer->Release();
				xmvPlayer = NULL;
				XEnableScreenSaver(true);

				m_FullscreenVideo.SetShow(false);
				m_Backdrop.SetShow(false);
				m_VideoList.SetEnable(true);
				m_VideoList.SetFocus(XUSER_INDEX_ANY);
				isFullscreen = false;

				ResetImages();
			}

			bHandled = TRUE;
		}else{
			NavigateBack(XUSER_INDEX_ANY);
			bHandled = TRUE;
		}
	} else if (hObjPressed == m_Select)
	{
		if(isFullscreen)
		{
			if(!controlsShown)
			{
				ShowPopUp(true);
				controlsShown = true;
			}
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_Pause)
	{	
		if(m_FullscreenVideo.IsPaused())
		{
			m_FullscreenVideo.Pause(false);
			XEnableScreenSaver(false);
			m_Pause.SetImage(strtowstr(pausePath).c_str());
			m_ButtonType.SetText(L"Pause");
		} else if (playspeed != PlaySpeed)
		{
			playspeed = PlaySpeed;
			xmvPlayer->SetPlaySpeed(playspeed);
			m_Pause.SetImage(strtowstr(pausePath).c_str());
			m_ButtonType.SetText(L"Pause");
		} else 
		{
			m_FullscreenVideo.Pause(true);
			XEnableScreenSaver(true);
			m_Pause.SetImage(strtowstr(playPath).c_str());
			m_ButtonType.SetText(L"Play");
		}	

		bHandled = TRUE;

	}else if (hObjPressed == m_Vol)
	{
		if (m_FullscreenVideo.IsMuted())
		{
			m_FullscreenVideo.Mute(false);
			m_FullscreenVideo.SetVolume(0.0f);
			m_Vol.SetImage(strtowstr(unMutePath).c_str());
			m_ButtonType.SetText(L"Mute");
		} else {
			m_FullscreenVideo.Mute(true);
			m_Vol.SetImage(strtowstr(mutePath).c_str());
			m_ButtonType.SetText(L"Unmute");
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_FF)
	{		
		if (playspeed == PlaySpeed)
		{
			playspeed = FastFowardSpeed;
			m_Pause.SetImage(strtowstr(playPath).c_str());	
		} else if (playspeed <= 8.0) {
			playspeed += 2;
		}
		xmvPlayer->SetPlaySpeed(playspeed);
		DebugMsg("ScnVideo","Video Speed %.1fx",playspeed);
		bHandled = TRUE;

	} else if (hObjPressed == m_RW)
	{		
		if (playspeed == PlaySpeed)
		{
			playspeed = -RewindSpeed;
			m_Pause.SetImage(strtowstr(playPath).c_str());
		} else {
			playspeed -= 2;
		}
		xmvPlayer->SetPlaySpeed(playspeed);
		DebugMsg("ScnVideo","Video Speed %.1fx",playspeed);
		bHandled = TRUE;

	} else if (hObjPressed == m_Stop)
	{
		xmvPlayer->Stop(XMEDIA_STOP_IMMEDIATE);
		XEnableScreenSaver(true);
		m_Pause.SetImage(strtowstr(playPath).c_str());
		m_Vol.SetImage(strtowstr(unMutePath).c_str());

		bHandled = TRUE;
	} else if (hObjPressed == m_AspectRatio)
	{
		DWORD dwIdSizeModeMain = 0;
		XUIElementPropVal propvalSizeMode;

		videoAR++;

		propvalSizeMode = SetAspectRatio(videoAR);

		XuiObjectGetPropertyId( m_FullscreenVideo.m_hObj, L"SizeMode", &dwIdSizeModeMain );
		XuiObjectSetProperty( m_FullscreenVideo.m_hObj, dwIdSizeModeMain, 0, &propvalSizeMode );

		if(videoAR == 0)
			m_ButtonType.SetText(L"Aspect Ratio - Full Screen w/ AR");
		else if(videoAR == 1)
			m_ButtonType.SetText(L"Aspect Ratio - Full Screen");
		else if(videoAR == 2)
			m_ButtonType.SetText(L"Aspect Ratio - Normal Size");


		bHandled = TRUE;
	} else if (hObjPressed == m_Subtitles)
	{

	}

	return S_OK;
}


HRESULT CScnVideo::InitializeChildren( void )
{
	HRESULT hr = NULL;

	//Scn Elements
	hr = GetChildById( L"VideoList", &m_VideoList );
	hasVideoList = hr == S_OK;
	hr = GetChildById( L"Video", &m_Video );
	hasVideo = hr == S_OK;
	hr = GetChildById( L"BackButton", &m_Back );
	hasBack = hr == S_OK;
	hr = GetChildById( L"Select", &m_Select );
	hasSelect = hr == S_OK;
	hr = GetChildById( L"MovieTitle", &m_VideoListTitle );
	hasVideoListTitle = hr == S_OK;
	hr = GetChildById( L"VideoCount", &m_VideoListCount );
	hasVideoListCount = hr == S_OK;
	hr = GetChildById( L"FullScreenVideo", &m_FullscreenVideo );
	hasFullscreenVideo = hr == S_OK;
	hr = GetChildById( L"FullScreenVideoBG", &m_Backdrop );
	hasFullScreenVideoBG = hr == S_OK;

	//Pop-Up Controls
	hr = GetChildById( L"Btn_Pause", &m_Pause );
	hasPause = hr == S_OK;
	hr = GetChildById( L"Btn_PrevChapter", &m_PrevChapter );
	hasPrevChapter = hr == S_OK;
	hr = GetChildById( L"Btn_RW", &m_RW );
	hasRW = hr == S_OK;
	hr = GetChildById( L"Btn_Stop", &m_Stop );
	hasStop = hr == S_OK;
	hr = GetChildById( L"Btn_FF", &m_FF );
	hasFF = hr == S_OK;
	hr = GetChildById( L"Btn_NextChapter", &m_NextChapter );
	hasNextChapter = hr == S_OK;
	hr = GetChildById( L"Btn_AR", &m_AspectRatio );
	hasAspectRatio = hr == S_OK;
	hr = GetChildById( L"Btn_Vol", &m_Vol );
	hasVol = hr == S_OK;
	hr = GetChildById( L"Btn_Subtitles", &m_Subtitles );
	hasSubtitles = hr == S_OK;
	hr = GetChildById( L"Btn_Info", &m_Info );
	hasInfo = hr == S_OK;
	hr = GetChildById( L"ButtonBackdrop", &m_ButtonBackdrop );
	hasButtonBackdrop = hr == S_OK;
	hr = GetChildById( L"ButtonType", &m_ButtonType );
	hasButtonType = hr == S_OK;

	//Audio Details
	hr = GetChildById( L"AudioChannels", &m_AudioChannels );
	hasAudioChannels = hr == S_OK;
	hr = GetChildById( L"AudioSR", &m_AudioSR );
	hasAudioSR = hr == S_OK;
	hr = GetChildById( L"AudioBR", &m_AudioBR );
	hasAudioBR = hr == S_OK;

	//Video Details
	hr = GetChildById( L"VideoWidth", &m_VideoWidth );
	hasVideoWidth = hr == S_OK;
	hr = GetChildById( L"VideoHeight", &m_VideoHeight );
	hasVideoHeight = hr == S_OK;
	hr = GetChildById( L"VideoFR", &m_VideoFR );
	hasVideoFR = hr == S_OK;
	hr = GetChildById( L"VideoDuration", &m_VideoDuration );
	hasVideoDuration = hr == S_OK;

	//Video AR Modes
	hr = GetChildById( L"Normal", &m_ARNormal );
	hr = GetChildById( L"StretchARC", &m_ARStretchARC );
	hr = GetChildById( L"Stretch", &m_ARStretch );


	return S_OK;
}

HRESULT CScnVideo::FillInfo(int cur)
{
	if(hasVideoListTitle)
	{
		LPCWSTR title = strtowstr(VideoManager::getInstance().getVideoName(cur)).c_str();
		m_VideoListTitle.SetText(title);
	}
	if(hasAudioChannels)
	{
		LPCWSTR audioChan = strtowstr(sprintfaA("%d",VideoManager::getInstance().getAudioInfo(cur).dwNumberOfChannels)).c_str();
		m_AudioChannels.SetText(audioChan);
	}
	if(hasAudioSR)
	{
		LPCWSTR audioSR = strtowstr(sprintfaA("%d Hz",VideoManager::getInstance().getAudioInfo(cur).dwSampleRate)).c_str();
		m_AudioSR.SetText(audioSR);
	}
	if(hasAudioBR)
	{
		LPCWSTR audioBR = strtowstr(sprintfaA("%d bit",VideoManager::getInstance().getAudioInfo(cur).dwBitsPerSample)).c_str();
		m_AudioBR.SetText(audioBR);
	}
	if(hasVideoWidth)
	{
		LPCWSTR videoW = strtowstr(sprintfaA("%d",VideoManager::getInstance().getVideoInfo(cur).dwWidth)).c_str();
		m_VideoWidth.SetText(videoW);
	}
	if(hasVideoHeight)
	{
		LPCWSTR videoH = strtowstr(sprintfaA("%d",VideoManager::getInstance().getVideoInfo(cur).dwHeight)).c_str();
		m_VideoHeight.SetText(videoH);
	}
	if(hasVideoFR)
	{
		LPCWSTR videoRate = strtowstr(sprintfaA("%.1f Frames/Sec",VideoManager::getInstance().getVideoInfo(cur).fFrameRate)).c_str();
		m_VideoFR.SetText(videoRate);
	}
	if(hasVideoDuration)
	{
		int Milliseconds = (int)VideoManager::getInstance().getVideoInfo(cur).dwClipDuration;
		int Hours = Milliseconds / (1000*60*60);
		int Minutes = (Milliseconds % (1000*60*60)) / (1000*60);
		int Seconds = ((Milliseconds % (1000*60*60)) % (1000*60)) / 1000;
	
		LPCWSTR videoDuration = strtowstr(sprintfaA("H %.2d  M %.2d  S %.2d",Hours,Minutes,Seconds)).c_str();

		m_VideoDuration.SetText(videoDuration);
	}
	if(hasVideoListCount)
	{
		LPCWSTR count = strtowstr(sprintfaA("%d of %d",cur + 1, totalVideos)).c_str();
		m_VideoListCount.SetText(count);
	}

	return S_OK;
}

HRESULT CScnVideo::ShowPopUp(bool show)
{
	m_Pause.SetShow(show);
	m_PrevChapter.SetShow(show);
	m_RW.SetShow(show);
	m_Stop.SetShow(show);
	m_FF.SetShow(show);
	m_NextChapter.SetShow(show);
	m_AspectRatio.SetShow(show);
	m_Vol.SetShow(show);
	m_Subtitles.SetShow(show);
	m_Info.SetShow(show);
	m_ButtonBackdrop.SetShow(show);
	m_ButtonType.SetShow(show);

	if(show)
		m_Pause.SetFocus(XUSER_INDEX_ANY);

	return S_OK;
}

HRESULT CScnVideo::ResetImages()
{
	m_Vol.SetImage(strtowstr(unMutePath).c_str());
	m_Pause.SetImage(strtowstr(pausePath).c_str());

	return S_OK;
}

XUIElementPropVal CScnVideo::SetAspectRatio(int type)
{
	DWORD dwIdSizeModeSource;
	XUIElementPropVal propvalSizeMode;

	if (type > 2)
	{
		videoAR = 0;
		type = 0;
	}

	switch ( type ) {
		case 0:
			//Centered Stretched AR
			XuiObjectGetPropertyId( m_ARStretchARC.m_hObj, L"SizeMode", &dwIdSizeModeSource );
			XuiObjectGetProperty( m_ARStretchARC.m_hObj, dwIdSizeModeSource, 0, &propvalSizeMode);
			break;
		case 1:
			//Stretched
			XuiObjectGetPropertyId( m_ARStretch.m_hObj, L"SizeMode", &dwIdSizeModeSource );
			XuiObjectGetProperty( m_ARStretch.m_hObj, dwIdSizeModeSource, 0, &propvalSizeMode);
			break;
		case 2:
			//Fullsize
			XuiObjectGetPropertyId( m_ARNormal.m_hObj, L"SizeMode", &dwIdSizeModeSource );
			XuiObjectGetProperty( m_ARNormal.m_hObj, dwIdSizeModeSource, 0, &propvalSizeMode);
			break;

	}

	return propvalSizeMode;
}