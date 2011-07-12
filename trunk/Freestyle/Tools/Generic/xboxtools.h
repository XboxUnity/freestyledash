#pragma once

#include "../HTTP/Base/MemoryBuffer.h"
#include "KernelExports.h"
#include "XamExports.h"
#include "./DDS/DDSTools.h"
#include "./BMP/BMPTools.h"
using namespace std;

// Xui Helper Macros
#define XuiInitControl(XuiControlId, XuiControlVar, StatePtr) \
	if(GetChildById(XuiControlId, &XuiControlVar) == S_OK) { \
		if(StatePtr != NULL) *StatePtr = true; \
	} else { \
		if(StatePtr != NULL) *StatePtr = false; \
	}

#define XuiInitNestedControl(XuiParentVar, XuiControlId, XuiControlVar, StatePtr) \
	if(XuiParentVar.GetChildById(XuiControlId, &XuiControlVar) == S_OK) { \
		if(StatePtr != NULL) *StatePtr = true; \
	} else { \
		if(StatePtr != NULL) *StatePtr = false; \
	}




// Gamerpic Defines
#define GAMERPIC_PITCH				0x100 // 256
#define GAMERPIC_DIM				64

typedef struct
{
	LPCWSTR title;
	int type;
	bool reboot;
} WAIT_INFO, *pWAIT_INFO;

typedef struct
{
	BYTE Blue;
	BYTE Green;
	BYTE Red;
	BYTE Unused;
} X8R8G8B8;

typedef struct 
{
	unsigned short Major;
	unsigned short Minor;
	unsigned short Build;
	unsigned short Qfe;
}XBOX_KRNL_VERSION;

enum Timers 
{
	TM_LOADING,
	TM_OVERSCAN,
	TM_LOADTITLE,
	TM_LOADPREVIEW,
	TM_LOADSS1,
	TM_LOADSS2,
	TM_LOADBACK,
	TM_CLOCK,
	TM_DISK,
	TM_OVERLAPPED,
	TM_COPYPROGRESS,
	TM_COPYSPEED,
	TM_FORCECHECK,
	TM_OPERATIONDONE,
	TM_FAN_SPEED,
	TM_LED_TEMP,
	TM_PROFILE,
	TM_STORAGE,
    TM_NANDFLASHERPROGRESS,
	TM_CopyDVD,
	TM_FILE_MANAGER,
	TM_MESSAGEBOX,
	TM_KEYBOARD,
	TM_TITLE,
	TM_SCAN,
	TM_CANCEL,
	TM_RENAME,
	TM_DELETE,
	TM_TIMEOUT,
	TM_GOBACKTRIGGER,
	TM_KAISIGNIN,
	TM_KAISIGNOUT
};

enum DvdSpeed 
{
    DVD_SPEED_2X  = 1,
    DVD_SPEED_5X  = 2,
    DVD_SPEED_8X  = 3,
    DVD_SPEED_12X = 4
};

typedef enum _MEDIA_TYPES {
	MEDIA_TYPE_TYPE_NONE			= 0x00,
	MEDIA_TYPE_GAME_XBOX_360		= 0x01,
	MEDIA_TYPE_GAME_XBOX_ORIGINAL	= 0x02,
	MEDIA_TYPE_UNKNOWN				= 0x03,
	MEDIA_TYPE_DVD_AUDIO			= 0x04,
	MEDIA_TYPE_DVD_MOVIE			= 0x05,
	MEDIA_TYPE_CD_VIDEO				= 0x06,
	MEDIA_TYPE_CD_AUDIO				= 0x07,
	MEDIA_TYPE_CD_DATA				= 0x08,
	MEDIA_TYPE_GAME_MOVIE_HYBRID	= 0x09,
	MEDIA_TYPE_DVD_HD				= 0x0A,
} MEDIA_TYPES;

typedef unsigned int    u32;


string GenerateTimestampMd5( void );
BOOL XeKeysGetConsoleTypeCustom(DWORD* ConsoleType);
DWORD XamInputGetDeviceStatsCustom( DWORD Index, PXINPUT_DEVICE_STATS DeviceStats );
bool IsValidIPAddressString( string& szIpAddress );
CONST CHAR* GetLoadedImageName();
string GetPasswordMask(unsigned int nSize, string szMask);
void GetLocaleMap(map<string, string>& locale);
DWORD SwapDWORD(DWORD Value);
WORD SwapWORD(WORD Value);
HRESULT ForcePlayerSignOut(int dwPlayerIndex );
HRESULT ForcePlayerSignIn(int dwPlayerIndex, XUID profileXuid);
WORD StringToVKey(string parameters);
LONG WINAPI UnHandleExceptionFilter(struct _EXCEPTION_POINTERS
*lpExceptionInfo);
void ReverseARGB(byte* pData,int pWidth, int pHeight,int stride);
void debugLog(const char* output);
string GetMacAddressString();
void ftpLog(const char* output);
void ShutdownXbox(void);
void HardRebootXbox(void);
string ScreenShot(ATG::D3DDevice *pDevice = NULL);
void Restart();
u32 resolveFunct(char* modname, u32 ord);
FARPROC ResolveFunction(char* ModuleName, UINT32 Ordinal);
HRESULT mountCon(CHAR* szDrive, const CHAR* szDevice, const CHAR* szPath);
HRESULT unmountCon(CHAR* szDrive, bool bSaveChanges = false);
int GetGamerTag(int playerIndex,char* ret);
string GetGamerId(int playerIndex);
HRESULT XamUserLogonHook(void* buffer, int bufferSize, void* unknown);
string GetLocalTimeAsI64String();
SYSTEMTIME TimeI64StringToSystemTime(string t);
wstring SystemTimeToLocalString(const SYSTEMTIME& st);
void PatchInJump(UINT32* Address, UINT32 Destination, BOOL Linked);

void UnixTimeToFileTime(time_t* pt, LPFILETIME pft);
void FileTimeToUnixTime(LPFILETIME pft, time_t* pt);
void UnixTimeToSystemTime(time_t* pt, LPSYSTEMTIME pst);
void SystemTimeToUnixTime(LPSYSTEMTIME pst, time_t* pt); 

SIZE_T GetAvailableRam();
long GetGamerPoints(int playerIndex);
bool IsUserSignedIn(int playerIndex);
bool SaveTextureToPNGFile(IDirect3DTexture9* pTexture, string pLocation);

void ConvertImageFromFileToPNGBuffer(string szFileName, LPBYTE * bytes, long * len);
HRESULT ConvertGamerPicToPNGBuffer(DWORD dwPlayerIndex, LPBYTE * fileBuffer, long* fileSize);
void ConvertImageInMemoryToPNG(string szDestFileName, BYTE * fileBuffer, DWORD fileSize);

void ConvertImageInMemoryToPNGBuffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len);

HRESULT XNotifyQueueUICustom( WCHAR * buffer );
DWORD XamLoaderMediaGetInfoCustom( MEDIA_TYPES * mediaType );
DWORD XamLoaderGetDvdTrayStateCustom( void );
int GetGamerPicture(int playerIndex,PBYTE ret,BOOL small,DWORD pitch,DWORD height);
bool InitNetwork();
// void DebugMsg(const char *format, ...) ; //MATTIE: replaced by XeDebug version
void FTPMsg(const char *format, ...) ;
void ExceptionToDebug(struct _EXCEPTION_POINTERS *eps);
void BackToNXE();
void SendXuiKeyStroke(WORD VirtualKey);
void RecursiveFileSearch(vector<string> * m_Paths, string source, string filetype);
void RecursiveFolderSearch(vector<string> * m_Paths, string source, string filetype);
HRESULT GetKernelVersion(XBOX_KRNL_VERSION * kernelVer);
