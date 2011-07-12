// xconfig.h
#ifndef _XCONFIG_H
#define _XCONFIG_H

#include "types.h"

//u16 XConfig_numEntr[] = {0x3, 0xA, 0xA, 0x31, 0x2, 0x2, 0xB, 0x8, 0x3, 0x5, 0x3}; // the number of entries in each category @ 9199

const char XConfig_names[][38] = { // names of categories
	"XCONFIG_STATIC_CATEGORY", 
	"XCONFIG_STATISTIC_CATEGORY", 
	"XCONFIG_SECURED_CATEGORY", 
	"XCONFIG_USER_CATEGORY", 
	"XCONFIG_XNET_MACHINE_ACCOUNT_CATEGORY", 
	"XCONFIG_XNET_PARAMETERS_CATEGORY", 
	"XCONFIG_MEDIA_CENTER_CATEGORY", 
	"XCONFIG_CONSOLE_CATEGORY", 
	"XCONFIG_DVD_CATEGORY", 
	"XCONFIG_IPTV_CATEGORY", 
	"XCONFIG_SYSTEM_CATEGORY"
};

enum { // category enumerator
	//XCONFIG_CATEGORY_TYPES
	XCONFIG_STATIC_CATEGORY = 0x0,					//_XCONFIG_STATIC_SETTINGS
	XCONFIG_STATISTIC_CATEGORY = 0x1,				//_XCONFIG_STATISTIC_SETTINGS
	XCONFIG_SECURED_CATEGORY = 0x2,					//_XCONFIG_SECURED_SETTINGS
	XCONFIG_USER_CATEGORY = 0x3,					//_XCONFIG_USER_SETTINGS
	XCONFIG_XNET_MACHINE_ACCOUNT_CATEGORY = 0x4,	//_XCONFIG_XNET_SETTINGS
	XCONFIG_XNET_PARAMETERS_CATEGORY = 0x5,			//_XCONFIG_XNET_SETTINGS
	XCONFIG_MEDIA_CENTER_CATEGORY = 0x6,			//_XCONFIG_MEDIA_CENTER_SETTINGS
	XCONFIG_CONSOLE_CATEGORY = 0x7, 				//_XCONFIG_CONSOLE_SETTINGS
	XCONFIG_DVD_CATEGORY = 0x8, 					//_XCONFIG_DVD_SETTINGS
	XCONFIG_IPTV_CATEGORY = 0x9, 					//_XCONFIG_IPTV_SETTINGS
	XCONFIG_SYSTEM_CATEGORY = 0xa,					//_XCONFIG_SYSTEM_SETTINGS
	XCONFIG_CATEGORY_MAX = 0xb,
};


#pragma warning(push)
#pragma warning(disable: 4201)  // unnamed struct/union
#pragma pack(push, 1)

/* **************************** _XCONFIG_SECURED_SETTINGS **************************** */
//XCONFIG_SECURED_ENTRIES
enum{
	XCONFIG_SECURED_DATA = 0x0,
	XCONFIG_SECURED_MAC_ADDRESS = 0x1,
	XCONFIG_SECURED_AV_REGION = 0x2,
	XCONFIG_SECURED_GAME_REGION = 0x3,
	XCONFIG_SECURED_DVD_REGION = 0x4,
	XCONFIG_SECURED_RESET_KEY = 0x5,
	XCONFIG_SECURED_SYSTEM_FLAGS = 0x6,
	XCONFIG_SECURED_POWER_MODE = 0x7,
	XCONFIG_SECURED_ONLINE_NETWORK_ID = 0x8,
	XCONFIG_SECURED_POWER_VCS_CONTROL = 0x9,
	XCONFIG_SECURED_ENTRIES_MAX = 0xa
};

// _XCONFIG_SECURED_SETTINGS macros for key pointers
#define XK_SECURED_1(x) &x.MACAddress			// key 0x1 6 bytes
#define XK_SECURED_2(x) &x.AVRegion				// key 0x2 4 bytes
#define XK_SECURED_3(x) &x.GameRegion			// key 0x3 2 bytes
#define XK_SECURED_4(x) &x.DVDRegion			// key 0x4 4 bytes
#define XK_SECURED_5(x) &x.ResetKey				// key 0x5 4 bytes
#define XK_SECURED_6(x) &x.OnlineNetworkID//??	// ??key 0x6 4 bytes?? key 0x8 4 bytes?? NOT SURE WHICH, 0x8 or 0x6
#define XK_SECURED_7(x) &x.PowerMode			// key 0x7 2 bytes
#define XK_SECURED_8(x) &x.SystemFlags //??		// ??key 0x6 4 bytes?? key 0x8 4 bytes?? NOT SURE WHICH, 0x8 or 0x6
#define XK_SECURED_9(x) &x.PowerVcsControl		// key 0x9 2 bytes

typedef struct _XCONFIG_POWER_MODE{ // used by _XCONFIG_SECURED_SETTINGS
	u8 VIDDelta;
	u8 Reserved;
} XCONFIG_POWER_MODE, *PXCONFIG_POWER_MODE;

typedef struct _XCONFIG_POWER_VCS_CONTROL{ // used by _XCONFIG_SECURED_SETTINGS
	u16 Configured : 1;
	u16 Reserved : 3;
	u16 Full : 4;
	u16 Quiet : 4;
	u16 Fuse : 4;
} XCONFIG_POWER_VCS_CONTROL, *PXCONFIG_POWER_VCS_CONTROL; // 2 bytes

typedef struct _XCONFIG_SECURED_SETTINGS{
	u32 Checksum; //0
	u32 Version; //4
	char OnlineNetworkID[4]; //??key 0x6 4 bytes?? key 0x8 4 bytes?? NOT SURE WHICH, 0x8 or 0x6
	char Reserved1[8]; //12
	char Reserved2[12]; //20
	u8 MACAddress[6]; // key 0x1 6 bytes
	char Reserved3[2]; //38
	u32 AVRegion; // key 0x2 4 bytes - 00 00 10 00 can/usa
	u16 GameRegion; // key 0x3 2 bytes - 0x00FF can/usa
	char Reserved4[6];//46
	u32 DVDRegion;// key 0x4 4 bytes - 0x00000001 can/usa
	u32 ResetKey;// key 0x5 4 bytes
	u32 SystemFlags;// ??key 0x6 4 bytes?? key 0x8 4 bytes?? NOT SURE WHICH, 0x8 or 0x6
	XCONFIG_POWER_MODE PowerMode;// key 0x07 2 bytes
	XCONFIG_POWER_VCS_CONTROL PowerVcsControl;// key 0x9 2 bytes
	char ReservedRegion[444];//68
} XCONFIG_SECURED_SETTINGS, *PXCONFIG_SECURED_SETTINGS; // XConfigSecuredSettings; 512 bytes


/* **************************** _XCONFIG_CONSOLE_SETTINGS **************************** */
//XCONFIG_CONSOLE_ENTRIES
enum{
	XCONFIG_CONSOLE_DATA = 0x0,
	XCONFIG_CONSOLE_SCREEN_SAVER = 0x1,
	XCONFIG_CONSOLE_AUTO_SHUT_OFF = 0x2,
	XCONFIG_CONSOLE_WIRELESS_SETTINGS = 0x3,
	XCONFIG_CONSOLE_CAMERA_SETTINGS = 0x4,
	XCONFIG_CONSOLE_PLAYTIMERDATA = 0x5,
	XCONFIG_CONSOLE_MEDIA_DISABLEAUTOLAUNCH = 0x6,
	XCONFIG_CONSOLE_KEYBOARD_LAYOUT = 0x7,
};

// _XCONFIG_CONSOLE_SETTINGS macros for key pointers
#define XK_CONSOLE_1(x) &x.ScreenSaver				// key 0x1 2 bytes
#define XK_CONSOLE_2(x) &x.AutoShutoff				// key 0x2 2 bytes
#define XK_CONSOLE_3(x) &x.WirelessSettings			// key 0x3 256 bytes
#define XK_CONSOLE_4(x) &x.CameraSettings			// key 0x4 4 bytes
#define XK_CONSOLE_5(x) &x.PlayTimerData			// key 0x5 20 bytes
#define XK_CONSOLE_6(x) &x.MediaDisableAutoLaunch	// key 0x6 2 bytes
#define XK_CONSOLE_7(x) &x.KeyboardLayout			// key 0x7 2 bytes

typedef struct _XCONFIG_PLAYTIMERDATA{ // used by _XCONFIG_CONSOLE_SETTINGS
	_ULARGE_INTEGER uliResetDate;
	u32 dwPlayTimerFrequency;
	u32 dwTotalPlayTime;
	u32 dwRemainingPlayTime;
} XCONFIG_PLAYTIMERDATA, *PXCONFIG_PLAYTIMERDATA;

typedef struct _XCONFIG_CONSOLE_SETTINGS{
	u32 CheckSum;
	u32 Version;
	s16 ScreenSaver; // key 0x1 2 bytes
	s16 AutoShutoff; // key 0x2 2 bytes
	u8 WirelessSettings[256]; // key 0x3 256 bytes
	u32 CameraSettings; // key 0x4 4 bytes
	u8 CameraSettingsReserved[28];
	XCONFIG_PLAYTIMERDATA PlayTimerData; // key 0x5 20 bytes
	s16 MediaDisableAutoLaunch; // key 0x6 2 bytes
	s16 KeyboardLayout; // key 0x7 2 bytes
} XCONFIG_CONSOLE_SETTINGS, *PXCONFIG_CONSOLE_SETTINGS; // XConfigConsoleSettings; 324 bytes


/* **************************** _XCONFIG_DVD_SETTINGS **************************** */
//XCONFIG_DVD_ENTRIES
enum{
	XCONFIG_DVD_ALL = 0x0,
	XCONFIG_DVD_VOLUME_ID = 0x1,
	XCONFIG_DVD_BOOKMARK = 0x2,
	XCONFIG_DVD_ENTRIES_MAX = 0x3
};

// _XCONFIG_DVD_SETTINGS macros for key pointers
#define XK_DVD_1(x) &x.VolumeId	// key 0x1 20 bytes
#define XK_DVD_2(x) &x.Data		// key 0x2 640 bytes

typedef struct _XCONFIG_DVD_SETTINGS{
	u32 Version;
	u8 VolumeId[20]; // key 0x1 20 bytes
	u8 Data[640]; // key 0x2 640 bytes
} XCONFIG_DVD_SETTINGS, *PXCONFIG_DVD_SETTINGS; // XConfigDvdSettings; 664 bytes


/* **************************** _XCONFIG_IPTV_SETTINGS **************************** */
//XCONFIG_IPTV_ENTRIES
enum{
	XCONFIG_IPTV_DATA = 0x0,
	XCONFIG_IPTV_SERVICE_PROVIDER_NAME = 0x1,
	XCONFIG_IPTV_PROVISIONING_SERVER_URL = 0x2,
	XCONFIG_IPTV_SUPPORT_INFO = 0x3,
	XCONFIG_IPTV_BOOTSTRAP_SERVER_URL = 0x4,
	XCONFIG_IPTV_ENTRIES_MAX = 0x5
};

// _XCONFIG_IPTV_SETTINGS macros for key pointers
#define XK_IPTV_1(x) &x.ServiceProviderName		// key 0x1 120 bytes
#define XK_IPTV_2(x) &x.ProvisioningServerURL	// key 0x2 128 bytes
#define XK_IPTV_3(x) &x.SupportInfo				// key 0x3 128 bytes
#define XK_IPTV_4(x) &x.BootstrapServerURL		// key 0x4 128 bytes

typedef struct _XCONFIG_IPTV_SETTINGS{
	u32 CheckSum;
	u32 Version;
	wchar_t	ServiceProviderName[60];  // key 0x1 120 bytes
	wchar_t	ProvisioningServerURL[64]; // key 0x2 128 bytes
	wchar_t	SupportInfo[64]; // key 0x3 128 bytes
	wchar_t	BootstrapServerURL[64]; // key 0x4 128 bytes
} XCONFIG_IPTV_SETTINGS, *PXCONFIG_IPTV_SETTINGS; // XConfigIPTVSettings; 512 bytes


/* **************************** _XCONFIG_MEDIA_CENTER_SETTINGS **************************** */
//XCONFIG_MEDIA_CENTER_ENTRIES
enum{
	XCONFIG_MEDIA_CENTER_DATA = 0x0,
	XCONFIG_MEDIA_CENTER_MEDIA_PLAYER = 0x1,
	XCONFIG_MEDIA_CENTER_XESLED_VERSION = 0x2,
	XCONFIG_MEDIA_CENTER_XESLED_TRUST_SECRET = 0x3,
	XCONFIG_MEDIA_CENTER_XESLED_TRUST_CODE = 0x4,
	XCONFIG_MEDIA_CENTER_XESLED_HOST_ID = 0x5,
	XCONFIG_MEDIA_CENTER_XESLED_KEY = 0x6,
	XCONFIG_MEDIA_CENTER_XESLED_HOST_MAC_ADDRESS = 0x7,
	XCONFIG_MEDIA_CENTER_SERVER_UUID = 0x8,
	XCONFIG_MEDIA_CENTER_SERVER_NAME = 0x9,
	XCONFIG_MEDIA_CENTER_SERVER_FLAG = 0xa,
	XCONFIG_MEDIA_ENTRIES_MAX = 0xb
};

// _XCONFIG_MEDIA_CENTER_SETTINGS macros for key pointers
#define XK_MC_1(x) &x.MediaPlayer			// key 0x1 20 bytes
#define XK_MC_2(x) &x.XeSledVersion			// key 0x2 10 bytes
#define XK_MC_3(x) &x.XeSledTrustSecret		// key 0x3 20 bytes
#define XK_MC_4(x) &x.XeSledTrustCode		// key 0x4 8 bytes
#define XK_MC_5(x) &x.XeSledHostID			// key 0x5 20 bytes
#define XK_MC_6(x) &x.XeSledKey				// key 0x6 1628 bytes
#define XK_MC_7(x) &x.XeSledHostMACAddress	// key 0x7 6 bytes
#define XK_MC_8(x) &x.ServerUUID			// key 0x8 16 bytes
#define XK_MC_9(x) &x.ServerName			// key 0x9 128 bytes
#define XK_MC_A(x) &x.ServerFlag			// key 0xa 4 bytes

typedef struct _XCONFIG_MEDIA_CENTER_SETTINGS{
	u32 CheckSum;
	u32 Version;
	char MediaPlayer[20];// key 0x1 20 bytes
	u8 XeSledVersion[10];// key 0x2 10 bytes
	u8 XeSledTrustSecret[20];// key 0x3 20 bytes
	u8 XeSledTrustCode[8];// key 0x4 8 bytes
	u8 XeSledHostID[20];// key 0x5 20 bytes
	u8 XeSledKey[1628];// key 0x6 1628 bytes
	u8 XeSledHostMACAddress[6];// key 0x7 6 bytes
	char ServerUUID[16];// key 0x8 16 bytes
	char ServerName[128]; // key 0x9 128 bytes
	char ServerFlag[4]; // key 0xa 4 bytes
} XCONFIG_MEDIA_CENTER_SETTINGS, *PXCONFIG_MEDIA_CENTER_SETTINGS; // XConfigMediaCenterSettings; 1868 bytes


/* **************************** _XCONFIG_SYSTEM_SETTINGS **************************** */
//XCONFIG_SYSTEM_ENTRIES
enum{
	XCONFIG_SYSTEM_ALL = 0x0,
	XCONFIG_SYSTEM_ALARM_TIME = 0x1,
	XCONFIG_SYSTEM_PREVIOUS_FLASH_VERSION = 0x2,
	XCONFIG_SYSTEM_ENTRIES_MAX = 0x3
};

// _XCONFIG_SYSTEM_SETTINGS macros for key pointers
#define XK_SYSTEM_1(x) &x.AlarmTime				// key 0x1  8 bytes
#define XK_SYSTEM_2(x) &x.PreviousFlashVersion	// key 0x2 4 bytes

typedef struct _XCONFIG_SYSTEM_SETTINGS{
	u32 Version;
	union _LARGE_INTEGER AlarmTime; // key 0x1  8 bytes
	u32 PreviousFlashVersion; // key 0x2 4 bytes
} XCONFIG_SYSTEM_SETTINGS, *PXCONFIG_SYSTEM_SETTINGS; // XConfigSystemSettings; 16 bytes


/* **************************** _XCONFIG_USER_SETTINGS **************************** */
//XCONFIG_USER_ENTRIES
enum{
	XCONFIG_USER_DATA = 0x0,
	XCONFIG_USER_TIME_ZONE_BIAS = 0x1,
	XCONFIG_USER_TIME_ZONE_STD_NAME = 0x2,
	XCONFIG_USER_TIME_ZONE_DLT_NAME = 0x3,
	XCONFIG_USER_TIME_ZONE_STD_DATE = 0x4,
	XCONFIG_USER_TIME_ZONE_DLT_DATE = 0x5,
	XCONFIG_USER_TIME_ZONE_STD_BIAS = 0x6,
	XCONFIG_USER_TIME_ZONE_DLT_BIAS = 0x7,
	XCONFIG_USER_DEFAULT_PROFILE = 0x8,
	XCONFIG_USER_LANGUAGE = 0x9,
	XCONFIG_USER_VIDEO_FLAGS = 0xa,
	XCONFIG_USER_AUDIO_FLAGS = 0xb,
	XCONFIG_USER_RETAIL_FLAGS = 0xc,
	XCONFIG_USER_DEVKIT_FLAGS = 0xd,
	XCONFIG_USER_COUNTRY = 0xe,
	XCONFIG_USER_PC_FLAGS = 0xf,
	XCONFIG_USER_SMB_CONFIG = 0x10,
	XCONFIG_USER_LIVE_PUID = 0x11,
	XCONFIG_USER_LIVE_CREDENTIALS = 0x12,
	XCONFIG_USER_AV_COMPOSITE_SCREENSZ = 0x13,
	XCONFIG_USER_AV_COMPONENT_SCREENSZ = 0x14,
	XCONFIG_USER_AV_VGA_SCREENSZ = 0x15,
	XCONFIG_USER_PC_GAME = 0x16,
	XCONFIG_USER_PC_PASSWORD = 0x17,
	XCONFIG_USER_PC_MOVIE = 0x18,
	XCONFIG_USER_PC_GAME_RATING = 0x19,
	XCONFIG_USER_PC_MOVIE_RATING = 0x1a,
	XCONFIG_USER_PC_HINT = 0x1b,
	XCONFIG_USER_PC_HINT_ANSWER = 0x1c,
	XCONFIG_USER_PC_OVERRIDE = 0x1d,
	XCONFIG_USER_MUSIC_PLAYBACK_MODE = 0x1e,
	XCONFIG_USER_MUSIC_VOLUME = 0x1f,
	XCONFIG_USER_MUSIC_FLAGS = 0x20,
	XCONFIG_USER_ARCADE_FLAGS = 0x21,
	XCONFIG_USER_PC_VERSION = 0x22,
	XCONFIG_USER_PC_TV = 0x23,
	XCONFIG_USER_PC_TV_RATING = 0x24,
	XCONFIG_USER_PC_EXPLICIT_VIDEO = 0x25,
	XCONFIG_USER_PC_EXPLICIT_VIDEO_RATING = 0x26,
	XCONFIG_USER_PC_UNRATED_VIDEO = 0x27,
	XCONFIG_USER_PC_UNRATED_VIDEO_RATING = 0x28,
	XCONFIG_USER_VIDEO_OUTPUT_BLACK_LEVELS = 0x29,
	XCONFIG_USER_VIDEO_PLAYER_DISPLAY_MODE = 0x2a,
	XCONFIG_USER_ALTERNATE_VIDEO_TIMING_ID = 0x2b,
	XCONFIG_USER_VIDEO_DRIVER_OPTIONS = 0x2c,
	XCONFIG_USER_MUSIC_UI_FLAGS = 0x2d,
	XCONFIG_USER_VIDEO_MEDIA_SOURCE_TYPE = 0x2e,
	XCONFIG_USER_MUSIC_MEDIA_SOURCE_TYPE = 0x2f,
	XCONFIG_USER_PHOTO_MEDIA_SOURCE_TYPE = 0x30,
	XCONFIG_USER_ENTRIES_MAX = 0x31
};

// _XCONFIG_USER_SETTINGS macros for key pointers
#define XK_USER_1(x) &x.TimeZoneBias				// key 0x1 4 bytes
#define XK_USER_2(x) &x.TimeZoneStdName				// key 0x2 4 bytes
#define XK_USER_3(x) &x.TimeZoneDltName				// key 0x3 4 bytes
#define XK_USER_4(x) &x.TimeZoneStdDate				// key 0x4 4 bytes
#define XK_USER_5(x) &x.TimeZoneDltDate				// key 0x5 4 bytes
#define XK_USER_6(x) &x.TimeZoneStdBias				// key 0x6 4 bytes
#define XK_USER_7(x) &x.TimeZoneDltBias				// key 0x7 4 bytes
#define XK_USER_8(x) &x.DefaultProfile				// key 0x8 8 bytes
#define XK_USER_9(x) &x.Language					// key 0x9 4 bytes
#define XK_USER_A(x) &x.VideoFlags					// key 0xa 4 bytes
#define XK_USER_B(x) &x.AudioFlags					// key 0xb 4 bytes
#define XK_USER_C(x) &x.RetailFlags					// key 0xc 4 bytes
#define XK_USER_D(x) &x.DevkitFlags					// key 0xd 4  bytes
#define XK_USER_E(x) &x.Country						// key 0xe 1 bytes
#define XK_USER_F(x) &x.ParentalControlFlags		// key 0xf 1 bytes
#define XK_USER_10(x) &x.SMBConfig					// key 0x10 256 bytes
#define XK_USER_11(x) &x.LivePUID					// key 0x11 8 bytes
#define XK_USER_12(x) &x.LiveCredentials			// key 0x12 16 bytes
#define XK_USER_13(x) &x.AvPackHDMIScreenSz			// key 0x13 4 bytes
#define XK_USER_14(x) &x.AvPackComponentScreenSz	// key 0x14 4 bytes
#define XK_USER_15(x) &x.AvPackVGAScreenSz			// key 0x15 4 bytes
#define XK_USER_16(x) &x.ParentalControlGame		// key 0x16 4 bytes
#define XK_USER_17(x) &x.ParentalControlPassword	// key 0x17 4 bytes
#define XK_USER_18(x) &x.ParentalControlMovie		// key 0x18 4 bytes
#define XK_USER_19(x) &x.ParentalControlGameRating	// key 0x19 4 bytes
#define XK_USER_1A(x) &x.ParentalControlMovieRating	// key 0x1a 4 bytes
#define XK_USER_1B(x) &x.ParentalControlHint		// key 0x1b 1 bytes
#define XK_USER_1C(x) &x.ParentalControlHintAnswer	// key 0x1c 32 bytes
#define XK_USER_1D(x) &x.ParentalControlOverride	// key 0x1d 32  bytes
#define XK_USER_1E(x) &x.MusicPlaybackMode			// key 0x1e 4 bytes
#define XK_USER_1F(x) &x.MusicVolume				// key 0x1f 4 bytes
#define XK_USER_20(x) &x.MusicFlags					// key 0x20 4 bytes
#define XK_USER_21(x) &x.ArcadeFlags				// key 0x21 4 bytes
#define XK_USER_22(x) &x.ParentalControlVersion		// key 0x22 4 bytes
#define XK_USER_23(x) &x.ParentalControlTV			// key 0x23 4 bytes
#define XK_USER_24(x) &x.ParentalControlTVRating	// key 0x24 4 bytes
#define XK_USER_25(x) &x.ParentalControlExplicitVideo// key 0x25 4 bytes
#define XK_USER_26(x) &x.ParentalControlExplicitVideoRating// key 0x26 4 bytes
#define XK_USER_27(x) &x.ParentalControlUnratedVideo// key 0x27 4 bytes
#define XK_USER_28(x) &x.ParentalControlUnratedVideoRating// key 0x28 4 bytes
#define XK_USER_29(x) &x.VideoOutputBlackLevels		// key 0x29 4 bytes
#define XK_USER_2A(x) &x.VideoPlayerDisplayMode		// key 0x2a 1 bytes
#define XK_USER_2B(x) &x.AlternateVideoTimingIDs	// key 0x2b 4 bytes
#define XK_USER_2C(x) &x.VideoDriverOptions			// key 0x2c 4 bytes
#define XK_USER_2D(x) &x.MusicUIFlags				// key 0x2d 4 bytes
#define XK_USER_2E(x) &x.VideoMediaSourceType		// key 0x2e 1 bytes
#define XK_USER_2F(x) &x.MusicMediaSourceType		// key 0x2f 1 bytes
#define XK_USER_30(x) &x.PhotoMediaSourceType		// key 0x30 1 bytes

typedef struct _XCONFIG_TIMEZONE_DATE{ // used by _XCONFIG_USER_SETTINGS
	u8 Month;
	u8 Day;
	u8 DayOfWeek;
	u8 Hour;
} XCONFIG_TIMEZONE_DATE, *PXCONFIG_TIMEZONE_DATE;

typedef struct _XCONFIG_USER_SETTINGS{
	u32 CheckSum;
	u32 Version;
	u32 TimeZoneBias; // key 0x1 4 bytes
	char TimeZoneStdName[4];// key 0x2 4 bytes
	char TimeZoneDltName[4];// key 0x3 4 bytes
	XCONFIG_TIMEZONE_DATE TimeZoneStdDate;// key 0x4 4 bytes
	XCONFIG_TIMEZONE_DATE TimeZoneDltDate;// key 0x5 4 bytes
	u32 TimeZoneStdBias;// key 0x6 4 bytes
	u32 TimeZoneDltBias;// key 0x7 4 bytes
	u64 DefaultProfile;// key 0x8 8 bytes
	u32 Language;// key 0x9 4 bytes
	u32 VideoFlags;// key 0xa 4 bytes
	u32 AudioFlags;// key 0xb 4 bytes
	u32 RetailFlags;// key 0xc 4 bytes
	u32 DevkitFlags;// key 0xd 4 bytes
	char Country;// key 0xe 1 bytes
	char ParentalControlFlags;// key 0xf 1 bytes
	u8 ReservedFlags[2];
	char SMBConfig[256];// key 0x10 256 bytes
	u64 LivePUID;// key 0x11 8 bytes
	char LiveCredentials[16];// key 0x12 16 bytes
	s16 AvPackHDMIScreenSz[2];// key 0x13 4 bytes
	s16 AvPackComponentScreenSz[2];// key 0x14 4 bytes
	s16 AvPackVGAScreenSz[2];// key 0x15 4 bytes (1440x900? = 0780 fbc8)
	u32 ParentalControlGame;// key 0x16 4 bytes
	u32 ParentalControlPassword;// key 0x17 4 bytes
	u32 ParentalControlMovie;// key 0x18 4 bytes
	u32 ParentalControlGameRating;// key 0x19 4 bytes
	u32 ParentalControlMovieRating;// key 0x1a 4 bytes
	char ParentalControlHint;// key 0x1b 1 bytes
	char ParentalControlHintAnswer[32];// key 0x1c 32 bytes
	char ParentalControlOverride[32];// key 0x1d 32  bytes
	u32 MusicPlaybackMode;// key 0x1e 4 bytes
	s32 MusicVolume;// key 0x1f 4 bytes
	u32 MusicFlags;// key 0x20 4 bytes
	u32 ArcadeFlags;// key 0x21 4 bytes
	u32 ParentalControlVersion;// key 0x22 4 bytes
	u32 ParentalControlTV;// key 0x23 4 bytes
	u32 ParentalControlTVRating;// key 0x24 4 bytes
	u32 ParentalControlExplicitVideo;// key 0x25 4 bytes
	u32 ParentalControlExplicitVideoRating;// key 0x26 4 bytes
	u32 ParentalControlUnratedVideo;// key 0x27 4 bytes
	u32 ParentalControlUnratedVideoRating;// key 0x28 4 bytes
	u32 VideoOutputBlackLevels;// key 0x29 4 bytes
	u8 VideoPlayerDisplayMode;// key 0x2a 1 bytes
	u32 AlternateVideoTimingIDs;// key 0x2b 4 bytes
	u32 VideoDriverOptions;// key 0x2c 4 bytes
	u32 MusicUIFlags;// key 0x2d 4 bytes
	char VideoMediaSourceType;// key 0x2e 1 bytes
	char MusicMediaSourceType;// key 0x2f 1 bytes
	char PhotoMediaSourceType;// key 0x30 1 bytes
} XCONFIG_USER_SETTINGS, *PXCONFIG_USER_SETTINGS; // XConfigUserSettings; 509 bytes


/* **************************** _XCONFIG_XNET_SETTINGS **************************** */
//XCONFIG_XNET_ENTRIES
enum{
	XCONFIG_XNET_ALL = 0x0,
	XCONFIG_XNET_DATA = 0x1,
	XCONFIG_XNET_ENTRIES_MAX = 0x2
};

// _XCONFIG_XNET_SETTINGS macros for key pointers
#define XK_XNET_1(x) &x.Data // key 0x01 492 bytes

typedef struct _XCONFIG_XNET_SETTINGS{
	u32 version;
	u8 Data[492]; // key 0x01 492 bytes
} XCONFIG_XNET_SETTINGS, *PXCONFIG_XNET_SETTINGS; // 496 bytes


/* **************************** _XCONFIG_STATISTIC_SETTINGS **************************** */
//XCONFIG_STATISTIC_ENTRIES
enum{
	XCONFIG_STATISTICS_DATA = 0x0,
	XCONFIG_STATISTICS_XUID_MAC_ADDRESS = 0x1,
	XCONFIG_STATISTICS_XUID_COUNT = 0x2,
	XCONFIG_STATISTICS_ODD_FAILURES = 0x3,
	XCONFIG_STATISTICS_HDD_SMART_DATA = 0x4,
	XCONFIG_STATISTICS_UEM_ERRORS = 0x5,
	XCONFIG_STATISTICS_FPM_ERRORS = 0x6,
	XCONFIG_STATISTICS_LAST_REPORT_TIME = 0x7,
	XCONFIG_STATISTICS_BUG_CHECK_DATA = 0x8,
	XCONFIG_STATISTICS_TEMPERATURE = 0x9,
	XCONFIG_STATISTICS_ENTRIES_MAX = 0xa
};

// _XCONFIG_STATISTIC_SETTINGS macros for key pointers
#define XK_STATISTIC_1(x) &x.XUIDMACAddress		// key 0x1 6 bytes
#define XK_STATISTIC_2(x) &x.XUIDCount			// key 0x2 4 bytes
#define XK_STATISTIC_3(x) &x.ODDFailures		// key 0x3 32 bytes
#define XK_STATISTIC_4(x) &x.HDDSmartData		// key 0x4 512 bytes
#define XK_STATISTIC_5(x) &x.UEMErrors			// key 0x5 100 bytes
#define XK_STATISTIC_6(x) &x.FPMErrors			// key 0x6 56 bytes - 9199 increased 0x38 to 0x60
#define XK_STATISTIC_7(x) &x.LastReportTime		// key 0x7 8 bytes
#define XK_STATISTIC_8(x) &x.BugCheckData		// key 0x8 101 bytes 
#define XK_STATISTIC_9(x) &x.TemperatureData	// key 0x9 200 bytes

typedef struct _XCONFIG_STATISTIC_SETTINGS{
	u32 CheckSum;
	u32 Version;
	char XUIDMACAddress[6]; // key 0x1 6 bytes
	char Reserved[2];
	u32 XUIDCount;// key 0x2 4 bytes
	u8 ODDFailures[32];// key 0x3 32 bytes
	u8 BugCheckData[101];// key 0x8 101  bytes
	u8 TemperatureData[200];// key 0x9 200 bytes
	char Unused[467];
	char HDDSmartData[512];// key 0x4 512 bytes
	char UEMErrors[100];// key 0x5 100 bytes
	char FPMErrors[96];// key 0x6 56 bytes/96 bytes 9199
	u64 LastReportTime;// key 0x7 8 bytes
} XCONFIG_STATISTIC_SETTINGS, *PXCONFIG_STATISTIC_SETTINGS; // XConfigStatisticSettings; 1496 bytes


/* **************************** _XCONFIG_STATIC_SETTINGS **************************** */
//XCONFIG_STATIC_ENTRIES
enum{
	XCONFIG_STATIC_DATA = 0x0,
	XCONFIG_STATIC_FIRST_POWER_ON_DATE = 0x1,
	XCONFIG_STATIC_SMC_CONFIG = 0x2,
	XCONFIG_STATIC_ENTRIES_MAX = 0x3
};

// _XCONFIG_STATIC_SETTINGS macros for key pointers
#define XK_STATIC_1(x) &x.FirstPowerOnDate // key 0x1 5 bytes
#define XK_STATIC_2(x) &x.SMCConfig // key 0x2 256 bytes

typedef struct _FanOverride{ // used by _XCONFIG_STATIC_SETTINGS // 01111111 <- normally disabled as 0x7F
	u8 Enable : 1; // says 7 is bit 0
	u8 Speed : 7; // says 0 bit 7
} FanOverride, *PFanOverride; // 1 byte

typedef struct _TempCalData{ // used by _XCONFIG_STATIC_SETTINGS
	u16 Gain;
	u16 Offset; 
} TempCalData, *PTempCalData; // 4 bytes

typedef struct _Temperature{ // used by _XCONFIG_STATIC_SETTINGS
	TempCalData Cpu;
	TempCalData Gpu;
	TempCalData Edram;
	TempCalData Board; // 16 bytes
} Temperature, *PTemperature;

typedef struct _Thermal{ // used by _XCONFIG_STATIC_SETTINGS
	u8 Cpu;
	u8 Gpu;
	u8 Edram; // 3 bytes
} Thermal, *PThermal;

typedef struct _Viper{ // used by _XCONFIG_STATIC_SETTINGS
	u8 GpuVoltageNotSetting : 1; // bit 7
	u8 MemoryVoltageNotSetting : 1; // bit 6 (bit 5 thru 0 unused
	//u8 unknown : 6; // 1 byte
	u8 GpuTarget;
	u8 MemoryTarget;
	u8 CheckSum;	
} Viper, *PViper; // 4 bytes

typedef struct _ThermalCalData{ // used by _XCONFIG_STATIC_SETTINGS
	Temperature therm; // 16 bytes
	char AnaFuseValue;
	Thermal SetPoint; // 3 bytes
	Thermal Overload; // 3 bytes
} ThermalCalData, *PThermalCalData;

typedef struct _SMCBlock{ // STATIC key 0x2 256 bytes // used by _XCONFIG_STATIC_SETTINGS
	u8 StructureVersion;
	u8 ConfigSource;
	char ClockSelect;
	FanOverride fanOrCpu; // 1 byte
	FanOverride fanOrGpu; // 1 byte
	char pad1[1];
	char unknownBits : 2;// 1 byte total for this bitfield
	char ScreenToolExecutionCount : 2;
	char ScreenToolFinished : 1;
	char ScreenToolStarted : 1;
	char UseTempCalDefaults : 1;
	char RadioEnable : 1; 
	char pad2[3];
	ThermalCalData mainTemp; // 23 bytes
	u8 pad3[1];
	Viper vFlags;
	u8 pad4[190];
	ThermalCalData backupTemp; // 23 bytes
	u8 pad5[3];
	u8 DoNotUse[2];
} SMCBlock, *PSMCBlock;

typedef struct _XCONFIG_STATIC_SETTINGS{
	u32 CheckSum; // *** see below ***
	u32 Version;
	char FirstPowerOnDate[5]; // key 0x1 5 bytes
	char reserved;
	SMCBlock SMCConfig;  // key 0x2 256 bytes
} XCONFIG_STATIC_SETTINGS, *PXCONFIG_STATIC_SETTINGS; // XConfigStaticSettings; 270 bytes

#pragma warning(pop)  // unnamed struct/union
#pragma pack(pop)

// this and internal SMCBlock checksums seem to be the only ones used at this point
//
// use this to calculate XCONFIG_STATIC_SETTINGS.CheckSum
// where data is a BYTE pointer to the full XCONFIG_STATIC_SETTINGS struct
//UINT XConfigStaticMainChecksumCalc(PBYTE data)
//{
//	UINT i, len, sum = 0;
//	data += 0x10;
//	for(i=0, len=252; i<len; i++)
//		sum += data[i]&0xFF;
//	sum = (~sum)&0xFFFF;
//	return ((sum&0xFF00)<<8)+((sum&0xFF)<<24);
//}

#endif // _XCONFIG_H

