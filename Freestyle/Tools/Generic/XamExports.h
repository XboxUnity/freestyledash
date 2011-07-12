#pragma once

#define DASHLAUNCH_CMD_PLAYDVD	7

typedef unsigned int u32;
typedef unsigned __int64 u64;

typedef struct _XINPUT_DEVICE_STATS {
   DWORD  BatteryLevel;
   DWORD  BatteryType;
   DWORD  SignalStrength;
   DWORD  DeviceType;
   DWORD  DevicePort;
   DWORD  DeviceVersion;
   int    Jumped;
} XINPUT_DEVICE_STATS, *PXINPUT_DEVICE_STATS; // 0x1C


typedef struct _XDASHLAUNCHDATA {
   UINT dwVersion; // 0x00-0x04
   UINT dwCommand; // 0x04-0x08
   UINT dwUserIndex; // 0x08-0x0C
   UCHAR Reserved[0x3F0]; // 0x0C-0x3FC
} XDASHLAUNCHDATA; // 0x3FC

typedef struct _XUSER_TITLE_PLAYED {
	DWORD		dwTitleId;
	DWORD		dwAchievementsPossible;
	DWORD		dwAchievementsEarned;
	DWORD		dwCredPossible;
	DWORD		dwCredEarned;
	WORD		wReservedAchievementCount;
	WORD		wUnused1;
	WORD		wAvatarAwardsPossible;
	WORD		wAvatarAwardsEarned;
	DWORD		dwReservedFlags;
	FILETIME	ftLastLoaded;
	WCHAR		wszTitleName[0x40];
} XUSER_TITLE_PLAYED, *PXUSER_TITLE_PLAYED;

typedef struct _XAMACCOUNTINFO {
	DWORD	dwReserved;
	DWORD	dwLiveFlags;
	WCHAR	szGamerTag[0x10];
	XUID	xuidOnline;
	DWORD	dwCachedUserFlags;
	DWORD	dwOnlineServiceNetworkID;
	BYTE	rgbPasscode[0x4];
	CHAR	szOnlineDomain[0x14];
	CHAR	szOnlineKerberosRealm[0x18];
	CHAR	rgbOnlineKey[0x10];
	CHAR	szUserPassportMembername[0x72];
	CHAR	rgcUserPassportPassword[0x20];
	CHAR	szOwnerPassportMembername[0x72];
} XAMACCOUNTINFO, *PXAMACCOUNTINFO;

typedef struct _PROFILEENUMRESULT {
	XUID			xuidOffline;
	XAMACCOUNTINFO	xai;
	DWORD			DeviceID;
} PROFILEENUMRESULT, *PPROFILEENUMRESULT;

typedef	void (*XNotifyQueueUI)(u64 stringId, u64 playerIndex, u64 r5, WCHAR* displayText, u64 r7);
typedef DWORD (*XamInputGetDeviceStats)(DWORD index, VOID* buffer);
typedef	DWORD (*XamLoaderGetDvdTrayState)(void);
typedef DWORD (*XamLoaderMediaGetInfo)(char*, char*);

extern "C" {
	HRESULT XamContentOpenFile(u32, char*, char*, u32, u32, u32, u32*);
	HRESULT XamContentClose(char*, u32*);
	HRESULT XamContentFlush(char*, u32*);

	HANDLE XamNotifyCreateListener(ULONGLONG qwAreas, ULONGLONG Unknown);
	HANDLE XamNotifyCreateListenerInternal(ULONGLONG qvAreas, DWORD System, DWORD Unknown);

	void __stdcall XamSetDvdSpindleSpeed(int Speed);
	DWORD XamUpdateGetCurrentSystemVersion();
	DWORD XamGetGameInfo(char*, u32, u32, u32);
	
	DWORD XamUserLogon( PXUID xuids, DWORD dwFlags, PXOVERLAPPED pOverlapped);
	DWORD XamUserCreateAchievementEnumerator( DWORD dwTitleId, DWORD dwUserIndex, XUID xuid, DWORD dwDetailFlags, DWORD dwStartingIndex, DWORD cItem, PDWORD pcbBuffer, PHANDLE ph );
	DWORD XamUserCreateTitlesPlayedEnumerator( DWORD r3, DWORD dwUserIndex, XUID xuid, DWORD dwStartingIndex, DWORD cItem, PDWORD pcbBuffer, PHANDLE ph );

	DWORD XamProfileCreateEnumerator( DWORD dwDetailFlags, PHANDLE ph );
	DWORD XamProfileEnumerate( HANDLE hEnum, DWORD r4, PVOID pvBuffer,PXOVERLAPPED pOverlapped);
	DWORD XamReadTileToTexture( DWORD r3, DWORD dwTitleId, DWORD dwImageId, DWORD dwUserIndex, PBYTE pbTextureBuffer, DWORD dwPitch, DWORD dwHeight, PXOVERLAPPED pOverlapped );

}