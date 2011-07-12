#pragma once

#include <xmp.h>

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Debug/Debug.h"

using namespace std;

class MusicInfo {
public:
	string Name;
	string Path;
	bool Played;
};

class CScnMusic : public CXuiSceneImpl
{
protected:

	CXuiList m_MusicList;
	CXuiList m_PlayList;
	CXuiControl m_Back;
	CXuiControl m_Next;
	CXuiControl m_Previous;
	CXuiControl m_Shuffle;
	CXuiControl m_Repeat;
	CXuiControl m_Play;
	CXuiTextElement m_PlayListTitle;
	

	string searchcmd;
	int playcount;
	int currentsong;
	string songname;
	bool Shuffle;
	bool Repeat;
	string basepath;

	// XMP Stuff
    XMP_HANDLE m_hXMPPlaylist;            // The music player playlist
    XMP_STATE m_XMPState;                // The current status of the music player


	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
        XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
    HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

	void GetList();
	void recursive(string searchpath);
	void PlaySong(bool skip);
	map<int, int> Played;

public:
	map<string, MusicInfo> Music;
    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CScnMusic, L"ScnMusic", XUI_CLASS_TABSCENE )
};