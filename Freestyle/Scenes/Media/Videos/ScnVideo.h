#pragma once

#include <xmedia2.h>
#include <d3dx9math.h>

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Debug/Debug.h"
#include "../../Abstracts/ConfigurableScene/ConfigurableScene.h"

using namespace std;

#define PlaySpeed 1.0f
#define FastFowardSpeed 2.0f
#define RewindSpeed 1.0f


class CScnVideo : public ConfigurableScene
{
protected:

	CXuiList m_VideoList;
	CXuiTextElement m_VideoListTitle, m_VideoListCount;
	CXuiVideo m_Video, m_FullscreenVideo;
	CXuiVideo *m_PFullscreenVideo;
	CXuiControl m_Back, m_Sort, m_Select, m_Play;
	CXuiElement m_Backdrop, m_VideoControls;
	bool hasVideoList, hasVideo, hasBack, hasSort, hasPlay, hasSelect, hasVideoListTitle, hasVideoListCount, hasFullscreenVideo, hasFullScreenVideoBG, hasControlPopUp;

	CXuiControl m_Pause, m_PrevChapter, m_RW, m_Stop, m_FF, m_NextChapter, m_AspectRatio, m_Subtitles, m_Info, m_Vol;
	CXuiElement m_ButtonBackdrop;
	CXuiTextElement m_ButtonType;
	bool hasFF, hasRW, hasPause, hasPrevChapter, hasStop, hasNextChapter, hasAspectRatio, hasSubtitles, hasInfo, hasVol, hasButtonBackdrop, hasButtonType;

	CXuiTextElement m_AudioChannels, m_AudioSR, m_AudioBR;
	bool hasAudioChannels, hasAudioSR, hasAudioBR;

	CXuiTextElement m_VideoWidth, m_VideoHeight, m_VideoFR, m_VideoDuration;
	bool hasVideoWidth, hasVideoHeight, hasVideoFR, hasVideoDuration;

	CXuiVideo m_ARNormal, m_ARStretchARC, m_ARStretch;

	float vol, playspeed;
	int active, totalVideos, videoAR;
	bool isFullscreen, controlsShown;
	string volume, playPath, pausePath, mutePath, unMutePath;
	IXMedia2XmvPlayer* xmvPlayer;
	XMEDIA_XMV_CREATE_PARAMETERS parameters;
	PXMEDIA_VIDEO_SCREEN pScreen;
	XMEDIA_VIDEO_SCREEN Screen;
	XMEDIA_VIDEO_SCREEN Screen2;

	D3DXVECTOR3 location;
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
	
	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_SET_FOCUS( OnNotifySetFocus )
	XUI_END_MSG_MAP()

    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL &bHandled );

	HRESULT InitializeChildren( void );
	HRESULT FillInfo(int cur);
	HRESULT ShowPopUp(bool toggle);
	HRESULT ResetImages();
	XUIElementPropVal SetAspectRatio(int type);


public:
    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnVideo, L"ScnVideo", XUI_CLASS_TABSCENE )
};