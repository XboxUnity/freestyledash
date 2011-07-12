#pragma once
//#include <xmp.h>
#include <map>
#include <direct.h>
#include <string>
//#include <d3dx9math.h>
//#include <xfilecache.h>
//#include <ctime>

#include "stdafx.h"

#include "ScnMusic.h"

#include "../../../Application/FreeStyleApp.h"
#include "../../../Tools/Settings/Settings.h"
#include "../../../Tools/Threads/copythread.h"

using namespace std;
#define TM_CheckState 1234

HRESULT CScnMusic::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnMusic","Display Music");

	GetChildById( L"MusicList", &m_MusicList );
	GetChildById( L"PlayList", &m_PlayList);
	GetChildById( L"BackButton", &m_Back );
	GetChildById( L"Play", &m_Play );
	GetChildById( L"Shuffle", &m_Shuffle );
	GetChildById( L"Repeat", &m_Repeat );
	GetChildById( L"Next", &m_Next );
	GetChildById( L"Previous", &m_Previous );
	GetChildById( L"PlayListTitle", &m_PlayListTitle );
	

    m_hXMPPlaylist = NULL;

	playcount = 0;
	currentsong = 0;
	Shuffle = false;
	Repeat = false;
	basepath = SETTINGS::getInstance().getMusicPath();
	if (basepath == "" )
	{
		basepath = "game:\\Music\\";
		SETTINGS::getInstance().setMusicPath(basepath);
	}
	GetList();
	return S_OK;
}

HRESULT CScnMusic::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_MusicList)
	{	
		int remove = 0;
		int count = m_MusicList.GetCurSel();
		DebugMsg("ScnMusic", "Getting Name from first list");
		string choice = wstrtostr(m_MusicList.GetText(count));
		for (int x = 0; x < playcount; x++)
		{
			if (strcmp(wstrtostr(m_PlayList.GetText(x)).c_str(), choice.c_str())== 0)
			{
				DebugMsg("ScnMusic", "Found Match Removing");
				if (x < playcount)
				{
					for (int y = x; y < playcount; y++)
					{
						m_PlayList.SetText(y, m_PlayList.GetText(y+1));
					}
				}
				playcount--;
				m_PlayList.DeleteItems(playcount, 1);
				remove = 1;
			}
		}
		if (remove == 0)
		{
			DebugMsg("ScnMusic", "Adding new item to play list");
			m_PlayList.InsertItems(playcount, 1);
			m_PlayList.SetText(playcount, strtowstr(choice).c_str());
			playcount++;
		}
		bHandled = TRUE;

	} else if (hObjPressed == m_Back)
	{
		DWORD dwStatus;

		// Stop the music
		XMPStop( NULL );

		// Wait for the music to stop
		dwStatus = XMPGetStatus( &m_XMPState );
		assert( dwStatus == ERROR_SUCCESS );

		while( m_XMPState != XMP_STATE_IDLE )
		{
			Sleep( 1 );
			dwStatus = XMPGetStatus( &m_XMPState );
			assert( dwStatus == ERROR_SUCCESS );
		}

		if( m_hXMPPlaylist )
		{
			// Delete the playlist
			dwStatus = XMPDeleteTitlePlaylist( m_hXMPPlaylist );
			assert( dwStatus == ERROR_SUCCESS );
		}
		bHandled = TRUE;
		NavigateBack();

	} else if (hObjPressed == m_Next)
	{		
		if (currentsong > playcount-1)
		{
			currentsong = 0;
		}
		PlaySong(true);
		bHandled = TRUE;

	} else if (hObjPressed == m_Previous)
	{		
		currentsong--;
		currentsong--;
		if (currentsong < 0)
		{
			currentsong = playcount-1;
		}
		PlaySong(true);
		bHandled = TRUE;

	} else if (hObjPressed == m_Shuffle)
	{
		if ( !Shuffle )
		{
			Shuffle = true;
			m_Shuffle.SetText(L"Shuffle ON");
		} else {
			Shuffle = false;
			m_Shuffle.SetText(L"Shuffle OFF");
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_Repeat)
	{
		if ( !Repeat )
		{
			Repeat = true;
			m_Repeat.SetText(L"Repeat ON");
		} else {
			Repeat = false;
			m_Repeat.SetText(L"Repeat OFF");
		}
		bHandled = TRUE;
	} else if (hObjPressed == m_Play)
	{
		string title;
		DebugMsg("ScnMusic", "Getting XMP Status");
		XMPGetStatus( &m_XMPState );
		switch ( m_XMPState )
		{
			case XMP_STATE_PAUSED:
				DebugMsg("ScnMusic", "Paused starting");
				title = sprintfaA("PlayList: Playing - %s", songname.c_str());
				m_PlayListTitle.SetText(strtowstr(title).c_str());
				XMPContinue( NULL );
				break;
			case XMP_STATE_PLAYING:
				DebugMsg("ScnMusic", "Playing pausing");
				title = sprintfaA("PlayList: Paused - %s", songname.c_str());
				m_PlayListTitle.SetText(strtowstr(title).c_str());
				XMPPause( NULL );
				break;
			default:
				DebugMsg("ScnMusic", "IDLE Starting Playing");
				PlaySong(false);
        }
		bHandled = TRUE;
	}
	return S_OK;
}
void CScnMusic::recursive(string searchpath)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = searchpath + "*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (FileExt(findFileData.cFileName) == "mp3")
			{
				MusicInfo Stuff;
				string tempname = sprintfaA("%s", findFileData.cFileName);
				DebugMsg("ScnMusic", "Found %s", tempname.c_str());
				tempname = TrimRightStr(tempname, ".mp3");
				Stuff.Path = sprintfaA("%s%s", searchpath.c_str(), findFileData.cFileName);
				Stuff.Played = false;
				
				int suc = 0;
				int count = 0;
				while (suc == 0)
				{
					count++;
					std::map<string, MusicInfo>::iterator itr = Music.find(tempname.c_str());
					if (itr == Music.end())
					{
						Stuff.Name = tempname;
						Music[tempname] = Stuff;
						suc = 1;
					} else {
						tempname = sprintfa("%s %d", tempname.c_str(), count);
					}
				}
			} else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				string temppath = sprintfaA("%s%s\\", searchpath.c_str(), findFileData.cFileName);
				recursive(temppath);
			}
		} while (FindNextFile(hFind, &findFileData));
	}FindClose(hFind);
}

void CScnMusic::GetList()
{
	DebugMsg("ScnMusic", "Loading List Information");
	int Count = m_PlayList.GetItemCount();
	m_PlayList.DeleteItems(0, Count);
	Count = m_MusicList.GetItemCount();
	DebugMsg("ScnMusic", "Deleteing old items");
	m_MusicList.DeleteItems(0, Count);
	int MusicNo = 0;
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));	
	DebugMsg("ScnMusic", "Finding .mp3 files");
	searchcmd = basepath + "*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
		
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (FileExt(findFileData.cFileName) == "mp3")
			{
				MusicInfo Stuff;
				string tempname = sprintfaA("%s", findFileData.cFileName);
				DebugMsg("ScnMusic", "Found %s", tempname.c_str());
				tempname = TrimRightStr(tempname, ".mp3");
				Stuff.Path = sprintfaA("%s%s", basepath.c_str(), findFileData.cFileName);
				Stuff.Played = false;
				int suc = 0;
				int count = 0;
				while (suc == 0)
				{
					count++;
					std::map<string, MusicInfo>::iterator itr = Music.find(tempname.c_str());
					if (itr == Music.end())
					{
						Stuff.Name = tempname;
						Music[tempname] = Stuff;
						suc = 1;
					} else {
						tempname = sprintfa("%s %d", tempname.c_str(), count);
					}
				}
			} else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				string searchpath = sprintfaA("%s%s\\", basepath.c_str(), findFileData.cFileName);
				recursive(searchpath);
			}
		} while (FindNextFile(hFind, &findFileData));
	} FindClose(hFind);

	m_MusicList.InsertItems(0, Music.size());
	MusicNo = 0;
	std::map<string, MusicInfo>::iterator itr = Music.begin();
	for(; itr != Music.end(); itr++)
	{
		m_MusicList.SetText(MusicNo, strtowstr(itr->first).c_str());
		MusicNo++;
	}
}

HRESULT CScnMusic::OnTimer(XUIMessageTimer *pTimer, BOOL &bHandled)
{
	if (pTimer->nId == TM_CheckState)
	{
		XMPGetStatus(&m_XMPState);
        if (m_XMPState == XMP_STATE_IDLE)
		{
			PlaySong(false);
		}
				
	}
	return S_OK;
}


void CScnMusic::PlaySong(bool skip)
{
	if (!skip)
	{
		if (Shuffle)
		{
			Played.clear();
			int y = 0;
			for (int x = 0; x < playcount; x++)
			{
				std::map<string, MusicInfo>::iterator itr = Music.find(wstrtostr(m_PlayList.GetText(x)).c_str());
				if (!itr->second.Played)
				{
					Played[y] = x;
					y++;
				}
			}
			srand( y );
			if (y == 0 && Repeat)
			{
				std::map<string, MusicInfo>::iterator itr = Music.begin();
				for (;itr != Music.end(); itr++)
				{
					itr->second.Played = false;
				}
				currentsong = rand() % playcount;
			} else if (y == 0 && !Repeat)
			{
				currentsong = playcount +1;
			} else {
				int index = rand() % y;
				currentsong = Played[index];
			}
		}
	}
	if (currentsong < playcount)
	{
		DebugMsg("ScnMusic", "Loading first song");
		LPCWSTR name = m_PlayList.GetText(currentsong);
		std::map<string, MusicInfo>::iterator itr = Music.find(wstrtostr(name).c_str());
		wstring path = strtowstr(itr->second.Path);
		itr->second.Played = true;

		DebugMsg("ScnMusic", "Song: %s, Path: %s", wstrtostr(name).c_str(), wstrtostr(path).c_str());
		XMP_SONGDESCRIPTOR SongList[] =
		{
			{
				path.c_str(),				// File path.
				name,                // Song title.
				L"",                                    // Song artist.
				L"",                                    // Song album.
				L"",                                    // Song album artist.
				L"",                                    // Genre.
				0,                                      // Track number in the album.
				69000,                                  // Duration in milliseconds.
				XMP_SONGFORMAT_MP3,                     // Format of the song.;
			},
		};
	
		const DWORD         NUM_SONGS = sizeof(SongList) / sizeof(SongList[0]) ;
		XMPGetStatus( &m_XMPState );

		DebugMsg("ScnMusic", "Creating Playlist");
		DWORD dwStatus = XMPCreateTitlePlaylist( SongList, NUM_SONGS, XMP_CREATETITLEPLAYLISTFLAG_NONE, L"MyPlaylist", NULL, &m_hXMPPlaylist );
		if (dwStatus != ERROR_SUCCESS)
		{
			DebugMsg("ScnMusic", "Error creating playlist");
		}
	
		// Set the playback behavior to be in order and repeat the entire playlist
		DebugMsg("ScnMusic", "Setting Playback behaviour");
		dwStatus = XMPSetPlaybackBehavior( XMP_PLAYBACKMODE_INORDER, XMP_REPEATMODE_NOREPEAT, 0, NULL );
		if (dwStatus != ERROR_SUCCESS)
		{
			DebugMsg("ScnMusic", "Error setting playback behaviour");
		}

		// Play the playlist
		DebugMsg("ScnMusic", "Playing Song %s from %s", wstrtostr(SongList[0].pwszTitle).c_str(), wstrtostr(SongList[0].pwszFilePath).c_str());
		songname = wstrtostr(SongList[0].pwszTitle).c_str();
		string title = sprintfaA("PlayList: Playing - %s", songname.c_str());
		m_PlayListTitle.SetText(strtowstr(title).c_str());
		dwStatus = XMPPlayTitlePlaylist( m_hXMPPlaylist, NULL, NULL );
		if (dwStatus != ERROR_SUCCESS)
		{
			DebugMsg("ScnMusic", "Error Playing Song");
		}
		XMPSetVolume(1.0f, NULL);
		currentsong++;
		if (Repeat && currentsong >= playcount)
		{
			currentsong = 0;
		}
		SetTimer(TM_CheckState, 5000);
	} else {
		KillTimer(TM_CheckState);
		DWORD dwStatus;

		// Stop the music
		XMPStop( NULL );

		// Wait for the music to stop
		dwStatus = XMPGetStatus( &m_XMPState );
		assert( dwStatus == ERROR_SUCCESS );

		while( m_XMPState != XMP_STATE_IDLE )
		{
			Sleep( 1 );
			dwStatus = XMPGetStatus( &m_XMPState );
			assert( dwStatus == ERROR_SUCCESS );
		}

		if( m_hXMPPlaylist )
		{
			// Delete the playlist
			dwStatus = XMPDeleteTitlePlaylist( m_hXMPPlaylist );
			assert( dwStatus == ERROR_SUCCESS );
		}
		m_PlayListTitle.SetText(L"PlayList: Idle");
		currentsong = 0;
	}
}

