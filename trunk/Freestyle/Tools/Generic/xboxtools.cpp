#include "stdafx.h"

#include "../../Application/FreestyleApp.h"
#include "xboxtools.h"
#include "../Debug/debug.h"
#include "tools.h"
#include "../Threads/ThreadLock.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../FileBrowser/FileBrowser.h"
#include "XamExports.h"
#include "./PNG/lodepng.h"
#include "../HTTP/Base/MemoryBuffer.h"
#include "./DDS/DDSTools.h"


#define SCREENSHOT_PATH "game:\\Data\\Screenshots\\"

EXTERN_C CONST CHAR* ExLoadedImageName;
CONST CHAR* GetLoadedImageName() { return ExLoadedImageName; }

// some function pointers
void (*XamLoaderLaunchTitle)(const char *, u32); // not really needed, just use xlaunchnewimage

BOOL XeKeysGetConsoleTypeCustom(DWORD* ConsoleType) {

	// Get our certificate
	XE_CONSOLE_CERTIFICATE cert; DWORD certSize = 0x1A8;
	XeKeysGetKey(0x36, &cert, &certSize);
	
	// Verify our sizes
	if(certSize != 0x1A8) return FALSE;
	if(cert.CertSize != 0x1A8) return FALSE;

	// Now we can load our console type
	*ConsoleType = cert.ConsoleType;
	return TRUE;
}

string GenerateTimestampMd5( void )
{
	SYSTEMTIME LocalSysTime;
	GetLocalTime( &LocalSysTime );
	string szFileName = sprintfaA("%04d%02d%02d%02d%02d%02d%02d", 
		LocalSysTime.wYear, 
		LocalSysTime.wMonth, 
		LocalSysTime.wDay, 
		LocalSysTime.wHour, 
		LocalSysTime.wMinute, 
		LocalSysTime.wSecond, 
		LocalSysTime.wMilliseconds
	);				

	BYTE m_Serial[0x10];
	ZeroMemory(m_Serial, 0x10);
	XeCryptMd5((BYTE*)szFileName.c_str(), szFileName.length(), NULL, 0, NULL, 0, m_Serial, 0x10);

	// Get our serial as a string
	CHAR serialStr[41]; serialStr[40] = 0; UINT outLen = 0x40;
	ZeroMemory(serialStr, 41);
	GetBytesString(m_Serial, 0x10, serialStr, &outLen);
	
	return (string)serialStr;
}
u32 resolveFunct(char* modname, u32 ord)
{
   UINT32 ptr32=0, ret=0, ptr2=0;
   ret = XexGetModuleHandle(modname, &ptr32); //xboxkrnl.exe xam.dll?
   //console.Format("%s - XexGetModuleHandle ret: %08x, ptr32: %08x\n", modname, ret, ptr32);
   if(ret == 0)
   {
      ret = XexGetProcedureAddress(ptr32, ord, &ptr2 );
      //console.Format("%s - XexGetProcedureAddress ret: %08x, ptr2: %08x\n", modname, ret, ptr2);
      if(ptr2 != 0)
         return ptr2;
   }
   return 0; // function not found
}

DWORD XamLoaderMediaGetInfoCustom( MEDIA_TYPES * mediaType )
{
	// Resolve the function (oridinal 419)
	XamLoaderMediaGetInfo xamLoaderMediaGetInfo = (XamLoaderMediaGetInfo)ResolveFunction("xam.xex", 419);
	if(xamLoaderMediaGetInfo == NULL)
		return S_FALSE;

	char r3[0xC];
	char r4[0x14];

	DWORD status = xamLoaderMediaGetInfo(r3, r4);

	const char szType = r3[3];
	DWORD dwMediaType = (DWORD)szType;

	*mediaType = (MEDIA_TYPES) dwMediaType;
	return status;
}

bool IsValidIPAddressString( string& szIpAddress )
{
	vector<string> mParts;
	StringSplit(szIpAddress, ".", &mParts);

	// Check to make sure our string has 4 parts, if not, it's not valid
	if(mParts.size() != 4) return false;

	// Check to make sure that each part is a valid number between 0 and 255
	for( unsigned int nCount = 0; nCount < mParts.size(); nCount++ )
	{
		string szPart = mParts.at(nCount);
		if(atoi(szPart.c_str()) < 0 || atoi(szPart.c_str()) > 255)
			return false;
	}

	return true;
}

string GetPasswordMask(unsigned int nSize, string szMask)
{
	string szTemp = "";
	for( unsigned int nCount = 0; nCount < nSize; nCount++ )
	{
		szTemp.append(szMask);
	}

	return szTemp;
}

HRESULT ForcePlayerSignIn(int dwPlayerIndex, XUID profileXuid)
{
	XUID xuids[4] = { INVALID_XUID, INVALID_XUID, INVALID_XUID, INVALID_XUID };

	if(dwPlayerIndex < 0 || dwPlayerIndex > 3)
		return S_FALSE;
	
	// Check to ensure that the dwPlayerIndex does not already contain a valid player
//	XUID playerXUID;
//	if(XUserGetXUID(dwPlayerIndex, &playerXUID) == ERROR_SUCCESS)
//		return S_FALSE;

	xuids[dwPlayerIndex] = profileXuid;

	DWORD result = XamUserLogon(xuids, 0x424, NULL);

	return (HRESULT)result;
}

HRESULT ForcePlayerSignOut(int dwPlayerIndex )
{
	if(dwPlayerIndex < 0 || dwPlayerIndex > 3)
		return S_FALSE;

	DWORD result;
	XUID playerXUID[4];

	for(int i = 0; i < 4; i++){
		result = XUserGetXUID(i, &playerXUID[i]);
		if(result != ERROR_SUCCESS)
			playerXUID[i] = INVALID_XUID;
	}

	playerXUID[dwPlayerIndex] = INVALID_XUID;
	result = XamUserLogon(playerXUID, 0x23, NULL);

	return result;

}

FARPROC ResolveFunction(char* ModuleName, UINT32 Ordinal) 
{	 
	HMODULE mHandle = GetModuleHandle(ModuleName);
	if(mHandle == NULL)
		return NULL;

	return GetProcAddress(mHandle, (LPCSTR)Ordinal);
}

WORD StringToVKey(string parameters)
{
	// Buttons
	if(strcmp("VK_PAD_A", parameters.c_str()) == 0)
		return VK_PAD_A;
	if(strcmp("VK_PAD_B", parameters.c_str()) == 0)
		return VK_PAD_B;
	if(strcmp("VK_PAD_X", parameters.c_str()) == 0)
		return VK_PAD_X;
	if(strcmp("VK_PAD_Y", parameters.c_str()) == 0)
		return VK_PAD_Y;
	if(strcmp("VK_PAD_START", parameters.c_str()) == 0)
		return VK_PAD_START;
	if(strcmp("VK_PAD_BACK", parameters.c_str()) == 0)
		return VK_PAD_BACK;
	if(strcmp("VK_PAD_BIGBUTTON", parameters.c_str()) == 0)
		return VK_PAD_BIGBUTTON;

	// Shoulder Buttons and Triggers
	if(strcmp("VK_PAD_RSHOULDER", parameters.c_str()) == 0)
		return VK_PAD_RSHOULDER;
	if(strcmp("VK_PAD_LSHOULDER", parameters.c_str()) == 0)
		return VK_PAD_LSHOULDER;
	if(strcmp("VK_PAD_RTRIGGER", parameters.c_str()) == 0)
		return VK_PAD_RTRIGGER;
	if(strcmp("VK_PAD_LTRIGGER", parameters.c_str()) == 0)
		return VK_PAD_LTRIGGER;
	
	// DPAD Buttons
	if(strcmp("VK_PAD_DPAD_UP", parameters.c_str()) == 0)
		return VK_PAD_DPAD_UP;
	if(strcmp("VK_PAD_DPAD_DOWN", parameters.c_str()) == 0)
		return VK_PAD_DPAD_DOWN;
	if(strcmp("VK_PAD_DPAD_LEFT", parameters.c_str()) == 0)
		return VK_PAD_DPAD_LEFT;
	if(strcmp("VK_PAD_DPAD_RIGHT", parameters.c_str()) == 0)
		return VK_PAD_DPAD_RIGHT;

	// LTHUMB CONTROLS
	if(strcmp("VK_PAD_LTHUMB_PRESS", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_PRESS;
	if(strcmp("VK_PAD_LTHUMB_UP", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_UP;
	if(strcmp("VK_PAD_LTHUMB_DOWN", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_DOWN;
	if(strcmp("VK_PAD_LTHUMB_LEFT", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_LEFT;
	if(strcmp("VK_PAD_LTHUMB_RIGHT", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_RIGHT;
	if(strcmp("VK_PAD_LTHUMB_UPLEFT", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_UPLEFT;
	if(strcmp("VK_PAD_LTHUMB_UPRIGHT", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_UPRIGHT;
	if(strcmp("VK_PAD_LTHUMB_DOWNLEFT", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_DOWNLEFT;
	if(strcmp("VK_PAD_LTHUMB_DOWNRIGHT", parameters.c_str()) == 0)
		return VK_PAD_LTHUMB_DOWNRIGHT;

	// RTHUMB CONTROLS
	if(strcmp("VK_PAD_RTHUMB_PRESS", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_PRESS;
	if(strcmp("VK_PAD_RTHUMB_UP", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_UP;
	if(strcmp("VK_PAD_RTHUMB_DOWN", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_DOWN;
	if(strcmp("VK_PAD_RTHUMB_LEFT", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_LEFT;
	if(strcmp("VK_PAD_RTHUMB_RIGHT", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_RIGHT;
	if(strcmp("VK_PAD_RTHUMB_UPLEFT", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_UPLEFT;
	if(strcmp("VK_PAD_RTHUMB_UPRIGHT", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_UPRIGHT;
	if(strcmp("VK_PAD_RTHUMB_DOWNLEFT", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_DOWNLEFT;
	if(strcmp("VK_PAD_RTHUMB_DOWNRIGHT", parameters.c_str()) == 0)
		return VK_PAD_RTHUMB_DOWNRIGHT;


	return 0;
}

void ShutdownXbox(void)
{
	HalReturnToFirmware(5);
}

void HardRebootXbox(void)
{
	HalReturnToFirmware(6);
}

void GetLocaleMap(map<string, string>& locale)
{
	locale.clear();

	locale["Australia (English)"] = "en-AU";
	locale["Belgium (Dutch)"] = "nl-BE";
	locale["Belgium (French)"] = "fr-BR";
	locale["Brazil (Portuguese)"] = "pt-BR";
	locale["Canada (English)"] = "en-CA";
	locale["Canada (French)"] = "fr-CA";
	locale["Czech Republic (Czech)"] = "cs-CZ";
	locale["Chile (Spanish)"] = "es-CL";
	locale["Colombia (Spanish)"] = "es-CO";
	locale["Danmark (Danish)"] = "da-DK";
	locale["Germany (German)"] = "de-DE";
	locale["Hong Kong (English)"] = "en-HK";
	locale["Spain (Spanish)"] = "es-ES";
	locale["Greece (Greek)"] = "el-GR";
	locale["France (French)"] = "fr-FR";
	locale["Hong Kong (Chinese)"] = "zh-HK";
	locale["India (English)"] = "en-IN";
	locale["Ireland (English)"] = "en-IE";
	locale["Italy (Italian)"] = "it-IT";
	locale["Japan (Japanese)"] = "ja-JP";
	locale["Korea (Korean)"] = "ko-KR";
	locale["Hungary (Hungarian)"] = "hu-HU";
	locale["Mexico (Spanish)"] = "es-MX";
	locale["Netherlands (Dutch)"] = "nl-NL";
	locale["New Zealand (English)"] = "en-NZ";
	locale["Norway (Norwegian)"] = "nb-NO";
	locale["Austria (German)"] = "de-AT";
	locale["Poland (Polish)"] = "pl-PL";
	locale["Portugal (Portuguese)"] = "pt-PT";
	locale["Russia (Russian)"] = "ru-RU";
	locale["Switzerland (German)"] = "de-CH";
	locale["Singapore (English)"] = "en-SG";
	locale["Slovakia (Slovak)"] = "sk-SK";
	locale["South Africa (English)"] = "en-ZA";
	locale["Switzerland (French)"] = "fr-CH";
	locale["Finland (Finnish)"] = "fi-FI";
	locale["Sweden (Swedish)"] = "sv-SE";
	locale["Taiwan (Chinese)"] = "zh-TW";
	locale["United Arab Emirates (English)"] = "en-AE";
	locale["United Kingdom (English)"] = "en-GB";
	locale["United States (English)"] = "en-US";
}

string GetMacAddressString()
{
	BYTE * dataBuf;
	u16 settingSize;
	ExGetXConfigSetting(0x2, 0x1, NULL, NULL, &settingSize);
	dataBuf = new BYTE[settingSize];
	ExGetXConfigSetting(0x2, 0x1, dataBuf, settingSize, NULL);
	string szMacAddress = sprintfaA("%02X%02X%02X%02X%02X%02X", dataBuf[0], dataBuf[1], dataBuf[2], dataBuf[3], dataBuf[4], dataBuf[5]);

	return szMacAddress;
}

HRESULT XamUserLogonHook(void* buffer, int bufferSize, void* unknown)
{
	DebugMsg("Test", "r3=%X, r4=%X, r5=%X", buffer, bufferSize, unknown);
	XUSER_SIGNIN_INFO info;

	memcpy(&info, buffer, bufferSize);

	char nameBuffer[17];
	memcpy(nameBuffer, info.szUserName, 16);
	nameBuffer[16] = 0;

	BYTE * firstByte = ((BYTE*)unknown);
//	nameaddress = buffer+0x18;

	DebugMsg("Test", "XUID:  %I64X", info.xuid);
	DebugMsg("Test", "FLAGS:  %X", info.dwInfoFlags);
	DebugMsg("Test", "STATE:  %X", info.UserSigninState);
	DebugMsg("Test", "GUEST NUM:  %X", info.dwGuestNumber);
	DebugMsg("Test", "SPONSOR IDX:  %X", info.dwSponsorUserIndex);
	DebugMsg("Test", "UserNAME:  %X", firstByte[0]);

	
	return 0;//XamUserLogon(buffer, bufferSize, unknown);
}

void PatchInJump(UINT32* Address, UINT32 Destination, BOOL Linked) {
	
	if(Destination & 0x8000) // If bit 16 is 1
		Address[0] = 0x3D600000 + (((Destination >> 16) & 0xFFFF) + 1); // lis 	%r11, dest>>16 + 1
	else
		Address[0] = 0x3D600000 + ((Destination >> 16) & 0xFFFF); // lis 	%r11, dest>>16

	Address[1] = 0x396B0000 + (Destination & 0xFFFF); // addi	%r11, %r11, dest&0xFFFF
	Address[2] = 0x7D6903A6; // mtctr	%r11

	if(Linked)
		Address[3] = 0x4E800421; // bctrl
	else
		Address[3] = 0x4E800420; // bctr

	__dcbst(0, Address);
	__sync();
};

DWORD XamInputGetDeviceStatsCustom( DWORD Index, PXINPUT_DEVICE_STATS DeviceStats )
{
	XamInputGetDeviceStats xamInputGetDeviceStats = (XamInputGetDeviceStats)ResolveFunction("xam.xex", 407);
	if(xamInputGetDeviceStats == NULL)
		return S_FALSE;

	ZeroMemory(DeviceStats, sizeof(XINPUT_DEVICE_STATS));
	
	return xamInputGetDeviceStats( Index, DeviceStats );
}

HRESULT XNotifyQueueUICustom( WCHAR * buffer )
{
	// Notify a message on screen
	XNotifyQueueUI xNotifyQueueUI = (XNotifyQueueUI)ResolveFunction("xam.xex", 656);
	if(xNotifyQueueUI == NULL){
		return S_FALSE;
	}

	xNotifyQueueUI(0x22, 0xFF, 1, buffer, 0);

	return S_OK;
}

DWORD XamLoaderGetDvdTrayStateCustom( void )
{

	XamLoaderGetDvdTrayState xamLoaderGetDvdTrayState = (XamLoaderGetDvdTrayState)ResolveFunction("xam.xex", 426);
	if(xamLoaderGetDvdTrayState == NULL){
		return S_FALSE;
	}

	return xamLoaderGetDvdTrayState();
}
		
				
/*
int GetGamerXUID(int playerIndex,PXUID ret)
{
	DWORD dwErr = XUserGetXUID(playerIndex,ret);
	
	return dwErr;
}*/
void Restart()
{
	// Code Updated to Launch currently running Xex instead of just guessing
	if(FileExists(DrivesManager::getInstance().getExecutablePath()))
	{
		XLaunchNewImage(DrivesManager::getInstance().getExecutablePath().c_str(), 0);
	}
}
void BackToNXE()
{
	if (FileExistsA("flash:\\dash.xex"))
	{
		XLaunchNewImage("flash:\\dash.xex", 0);
	}
}

string ScreenShot(ATG::D3DDevice *pDevice)
{ 
	string ss_base = sprintfa("%s", SCREENSHOT_PATH);
	string szFilePath = "";

	try
	{	
		HRESULT hr = S_OK;

		if(pDevice == NULL)
			pDevice = ATG::Application::m_pd3dDevice;

		// Make Screenshots Directory
		RecursiveMkdir(ss_base);

		// Get Screenshot File Name
		SYSTEMTIME LocalSysTime;
		GetLocalTime( &LocalSysTime );
		string szFileName = sprintfaA("screenshot.%04d%02d%02d%02d%02d%02d%02d.png", 
			LocalSysTime.wYear, 
			LocalSysTime.wMonth, 
			LocalSysTime.wDay, 
			LocalSysTime.wHour, 
			LocalSysTime.wMinute, 
			LocalSysTime.wSecond, 
			LocalSysTime.wMilliseconds
		);
		// Form Full File Path
		szFilePath = ss_base + szFileName;

		// Get our display information
		BYTE displayInfo[0x58];
		VdGetCurrentDisplayInformation(displayInfo);

		// Get some information about our frame buffer
		BYTE* frameBufferPtr = (BYTE*)(0xDFFFF000 + *((UINT*)0x7FC86110));
		UINT width = *((UINT*)(displayInfo + 0x10));
		UINT height = *((UINT*)(displayInfo + 0x14));
		UINT size = (height * width) * 4;
		D3DFORMAT format = (((*((UINT*)0x7FC86104) >> 24) & 7) == 0) ? 
			D3DFMT_A8R8G8B8 : D3DFMT_A2R10G10B10;

		// Pad our sizes if needed
		UINT vWidth = width; UINT vHeight = height;
		if(vWidth % 128 != 0) vWidth += (128 - vWidth % 128);
		if(vHeight % 128 != 0) vHeight += (128 - vHeight % 128);
		UINT vSize = (vWidth * vHeight) * 4;

		// Untile our frame buffer
		void* untiledFrameBuffer = malloc(vSize);
		XGUntileSurface(untiledFrameBuffer, width * 4, NULL, 
			frameBufferPtr, width, height, NULL, 4);

		// Fix up some colors depending on format
		if(format == D3DFMT_A8R8G8B8)
			for(UINT x = 0; x < vSize / 4; x++)
				((X8R8G8B8*)untiledFrameBuffer)[x].Unused = 0xFF;

		// Display some information
		DebugMsg("Screenshot", "Frame Buffer Info:  Width: %d, Height: %d, Size: 0x%X, Format: %s", 
			width, height, size, (format == D3DFMT_A8R8G8B8) ? "A8R8G8B8" : "A2R10G10B10");

		BYTE * fileBufferData = (BYTE*)malloc(vSize + 0x80);
		memcpy((BYTE*)fileBufferData, (BYTE*)DxtHeader, 0x80);
		memcpy(((BYTE*)fileBufferData) + 0x80, (BYTE*)untiledFrameBuffer, vSize);

		// Create Temporary Texture from file data
		IDirect3DTexture9 * pTexture;
		
		hr = D3DXCreateTextureFromFileInMemoryEx(pDevice, fileBufferData, (size + 0x80), width, height, 1, D3DUSAGE_CPU_CACHED_MEMORY , D3DFMT_UNKNOWN, 0, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture);
		if(hr == S_OK)
		{
			SaveTextureToPNGFile(pTexture, szFilePath);
			XNotifyQueueUICustom(strtowchar(sprintfaA("Screenshot:  %s saved.", szFileName.c_str())));
			DebugMsg("Screenshot", "Screenshot Saved:  %s", szFilePath.c_str());
		}
		else
		{
			XNotifyQueueUICustom(strtowchar(sprintfaA("Screenshot failed to save.")));
			DebugMsg("Screenshot", "Failed to save screenshot.");
		}

		pTexture->Release();
		pTexture = NULL;

		// Free our allocated buffers
		free(untiledFrameBuffer);
		free(fileBufferData);
	}
	catch(...)
	{
		XNotifyQueueUICustom(strtowchar(sprintfaA("Screenshot failed to save.")));
		DebugMsg("Screenshot", "Failed to save screenshot.");
	}

	return szFilePath;
}


bool SaveTextureToPNGFile(IDirect3DTexture9* pTexture, string pLocation)
{
	/*LPD3DXBUFFER buf;
	FILE *file;
	HRESULT hr = D3DXSaveTextureToFileInMemory(&buf,D3DXIFF_PNG,pTexture,NULL);
	fopen_s(&file,pLocation.c_str(), "wb");
	fwrite((VOID*)buf->GetBufferPointer(), buf->GetBufferSize(), 1, file);
	fclose(file);*/
	HRESULT hr = D3DXSaveTextureToFile(pLocation.c_str(),D3DXIFF_PNG,pTexture,NULL);
	return hr ==D3D_OK;
}
void ReverseARGB(byte* pData,int pWidth, int pHeight,int stride)
{
//	DebugMsg("Working with %d x %d with %d stride",pWidth,pHeight);
	char *tgaData2;
	pWidth= stride / 4;
	//int heightIncrease = 3;
	//int widthIncrease = 9;
	int addToLoop = 0;//10752+(256+256);//;
	if(pWidth == 1280 && pHeight == 720)
	{
		
	//	addToLoop = (640*16)+(320*16)+(160*16)+(64*16)+(32*16)/*This one did no changed anything*/ +(32*16);
		//19456
	}
	else
	{
		if(pWidth == 1360 && pHeight == 768)
		{
			//addToLoop =0/* (368*32)+(16*16) or (736*16)+(lo16*16)*/;
		}
	}
	int size = ((pWidth * pHeight) + addToLoop) *4;
	tgaData2 = new char[size];
	memcpy(tgaData2,pData,size);
	int y =0;
	for(int x=0;x<(pWidth*pHeight)+addToLoop;x++)
	{
		int start = x*4;
		pData[start] = tgaData2[start +3];
		pData[start+1] = tgaData2[start +2];
		pData[start+2] = tgaData2[start +1];
		pData[start+3] = tgaData2[start];
		y++;
	}

}
bool SaveTextureToTGAFile(string pLocation, short pWidth, short pHeight, byte* pData,bool invertARGBChannel,bool revertPicture)
{
	FILE * file;
	char *tgaData;
	char *tgaData2;
	tgaData = new char[4*pWidth*pHeight];
	tgaData2 = new char[4*pWidth*pHeight];

	
	memcpy(tgaData,pData,pWidth*pHeight*4);
	memcpy(tgaData2,pData,pWidth*pHeight*4);
	
	if(invertARGBChannel)
	{
		int y =0;
		for(int x=0;x<pWidth*pHeight;x++)
		{
			int start = x*4;
			//int start2 = y*4;
			tgaData[start] = tgaData2[start +3];
			tgaData[start+1] = tgaData2[start +2];
			tgaData[start+2] = tgaData2[start +1];
			tgaData[start+3] = tgaData2[start];
			y++;
		}
		tgaData = tgaData2;
	}
	if(revertPicture)
	{
		int y =0;
		tgaData2 = new char[4*pWidth*pHeight];
		for(int x=(pWidth*pHeight)-1;x>=0;x--)
		{
			int start = x*4;
			int start2 = y*4;
			tgaData2[start] = tgaData[start2];
			tgaData2[start+1] = tgaData[start2 +1];
			tgaData2[start+2] = tgaData[start2 +2];
			tgaData2[start+3] = tgaData[start2 + 3];
			y++;
		}
		tgaData = tgaData2;
	}
	
	tgaData2=tgaData;
	/*
*/
    // Check to see if the file can be opened for reading
    file = NULL;
	
	fopen_s(&file,pLocation.c_str(), "wb");

	byte hiW = (byte)(pWidth >>8);
	byte loW = (byte)(pWidth & 255);
	byte hiH = (byte)(pHeight >>8);
	byte loH = (byte)(pHeight & 255);
	
	char tgaheader[18]={0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,loW,hiW,loH,hiH,0x20,0x08};

	fwrite(tgaheader,18,1,file);
	fwrite(tgaData, 4*pWidth*pHeight, 1, file);
	fclose(file);
	return true;
}

void ConvertImageToDXT5(string sourcePath)
{
	//overloaded function
	
	HRESULT retVal = NULL;
	vector<string> noType;

	StringSplit(sourcePath, ".", &noType);

	string noTypeName = noType.at(0);
	noType.clear();
	
	string szFileName = sourcePath;
	string szDestName = noTypeName + ".dds";


	IDirect3DTexture9 * pTexture;

	//Load image into a file buffer
	FILE * file;
	BYTE * buffer;
	DWORD fileLen;
	
	//Open file for reading
	fopen_s(&file, szFileName.c_str(), "rb");
	if(!file) { 
		DebugMsg("TextureCache", "Function failed to open file.");
		return;
	} 
	
	//Get file length 
	fseek(file, 0, SEEK_END); 
	fileLen = ftell(file); 
	fseek(file, 0, SEEK_SET); 
	//Allocate memory
	buffer = (BYTE *) malloc(fileLen + 1); 
	if(!buffer) { 
		DebugMsg("TextureCache", "Memory Error- Unable to allocate adequate buffer.");
		fclose(file); 
		return;
	} 
	fread(buffer, 1, fileLen, file); 
	fclose(file); 



	// Load File To Direct3D DXT5 Texture w/ 1 mipmap
	retVal = D3DXCreateTextureFromFileInMemoryEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		buffer,
		fileLen,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_DXT5,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);

	if(retVal == D3DXERR_INVALIDDATA)
	{
		// Occassionally Direct 3D will failed a texture create call due to invalid image data (xex problem?)
		XUIImageInfo imgInfo;

		imgInfo.MipLevels = 1;
		imgInfo.ImageFileFormat = D3DFMT_DXT5;

		retVal = XuiCreateTextureFromPNG(
			buffer, 
			fileLen,
			CFreestyleUIApp::getInstance().getXuiDevice(),
			&pTexture,
			&imgInfo
		);
	}

	//Save texture to dds file
	if( retVal == S_OK )
		retVal = D3DXSaveTextureToFile(szDestName.c_str(), D3DXIFF_DDS, pTexture, NULL);

	//Release Texture
	if(pTexture != NULL)
		pTexture->Release();

	pTexture = NULL;

	delete [] buffer;

	//Remove original
	_unlink(szFileName.c_str());

}

//Pathname contains the filetype
void ConvertImageToDXT5(string sourcePath, string destPath) {

	// Load image into a file buffer
	FILE* fHandle;
	BYTE* buffer;
	DWORD fileLen;
	
	// Open file for reading
	fopen_s(&fHandle, sourcePath.c_str(), "rb");
	if(!fHandle) { 
		return;
	} 
	
	// Get file length 
	fseek(fHandle, 0, SEEK_END); 
	fileLen = ftell(fHandle); 
	fseek(fHandle, 0, SEEK_SET);

	// Read file data
	buffer = (BYTE*)malloc(fileLen); 
	fread(buffer, 1, fileLen, fHandle); 
	fclose(fHandle); 

	// Convert in memory
	ConvertImageInMemoryToDXT5(destPath, buffer, fileLen);

	// Free our buffer
	free(buffer);
}

WORD SwapWORD(WORD Value) {
	return (((Value & 0xFF00) >> 8) | ((Value & 0xFF) << 8));
}

DWORD SwapDWORD(DWORD Value) {
	return (((Value & 0xFF000000) >> 24) | ((Value & 0xFF0000) >> 8) | 
		((Value & 0xFF00) << 8) | ((Value & 0xFF) << 24));
}

//Pathname contains the filetype
void ConvertImageInMemoryToPNG(string szDestFileName, BYTE * fileBuffer, DWORD fileSize) {
	
	// Check file size
	if (fileSize == 0) {
		DebugMsg("ConvertToPNG", "Bailing since filesize is 0");
		return;
	}

	// Convert our dds in memory
	BYTE* pngData = NULL; long pngSize = 0;
	ConvertImageInMemoryToPNGBuffer(fileBuffer, fileSize, &pngData, &pngSize);

	// Write to file if its valid
	if(pngSize != 0) {

		FILE * fHandle;
		fopen_s(&fHandle, szDestFileName.c_str(), "wb");
		fwrite(pngData, pngSize, 1, fHandle);
		fclose(fHandle);

		// Free our data
		free(pngData);
	}
}

void ConvertImageFromFileToPNGBuffer(string szFileName, LPBYTE * bytes, long * len) {

	if(!FileExists(szFileName))
		return;

	ATG::Timer m_Timer;

	double timeA = m_Timer.GetAbsoluteTime();

	IDirect3DTexture9 * pTexture;
	HRESULT retVal = D3DXCreateTextureFromFileEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		szFileName.c_str(),
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_LIN_A8R8G8B8,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);

	float timeB = m_Timer.GetAbsoluteTime();
	DebugMsg("Test", "Texture Creation:  %4.2f", (timeB- timeA));

	if(retVal == S_OK) {
		timeA = m_Timer.GetAbsoluteTime();

		// Get our level desc
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		// Now lock our data
		D3DLOCKED_RECT lock;
		RECT rect = {0, 0, desc.Width, desc.Height};
		pTexture->LockRect(0, &lock, &rect, D3DLOCK_READONLY);
	
		//Read our data
		DWORD dataLen = lock.Pitch * desc.Height;
		DWORD * dataBuffer = (DWORD*)malloc( dataLen );
		DWORD * address = (DWORD*)lock.pBits;

		
		// Manipulate Data
		unsigned int cnt = 0;
		for(DWORD y = 0; y < desc.Height; y++) {
			for(DWORD x = 0; x < desc.Width; x++) {

				// Fix this code to work with a byte array instead of vector
				DWORD cp = (y * lock.Pitch) + (x * 4);

				DWORD * temp = (DWORD*)(address + (cp / 4));
				DWORD * dest = (DWORD*)(dataBuffer + cnt);
				*dest = *temp >> 24 | *temp << 8;
				cnt++;
			}
		}
		
		// Unlock our texture
		pTexture->UnlockRect(0);

		timeB = m_Timer.GetAbsoluteTime();
		DebugMsg("Test", "Texture Lock Time:  %4.2f", (timeB- timeA));

		timeA = m_Timer.GetAbsoluteTime();
		// Encode the data and have output placed in buffers
		LodePNG_encode((unsigned char**)bytes, (size_t*)len, (BYTE*)dataBuffer, desc.Width, desc.Height, 6, 8);

		timeB = m_Timer.GetAbsoluteTime();
		DebugMsg("Test", "Encoding Time:  %4.2f", (timeB- timeA));
		// Free temporary dataBuffer
		free(dataBuffer);
	}else {
		DebugMsg("ConvertImageInMemoryToPNGBuffer", 
			"Conversion To PNG From Memory Failed. [%X]", retVal);		
	}

	// Release our Texture
	if(pTexture != NULL)
		pTexture->Release();

}

void ConvertImageInMemoryToPNGBuffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE * bytes, long* len) {

	// Set our initial values
	*bytes = NULL;
	*len = 0;

	// Check file size
	if (fileSize == 0) {
		DebugMsg("ConvertToPNG", "Bailing since filesize is 0");
		return;
	}

	ATG::Timer m_Timer;

	double timeA = m_Timer.GetAbsoluteTime();

	IDirect3DTexture9 * pTexture;
	HRESULT retVal = D3DXCreateTextureFromFileInMemoryEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		fileBuffer,
		fileSize,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_LIN_A8R8G8B8,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);

	float timeB = m_Timer.GetAbsoluteTime();
	DebugMsg("Test", "Texture Creation:  %4.2f", (timeB- timeA));

	if(retVal == S_OK) {
		timeA = m_Timer.GetAbsoluteTime();

		// Get our level desc
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		// Now lock our data
		D3DLOCKED_RECT lock;
		RECT rect = {0, 0, desc.Width, desc.Height};
		pTexture->LockRect(0, &lock, &rect, D3DLOCK_READONLY);
	
		//Read our data
		DWORD dataLen = lock.Pitch * desc.Height;
		DWORD * dataBuffer = (DWORD*)malloc( dataLen );
		DWORD * address = (DWORD*)lock.pBits;

		
		// Manipulate Data
		unsigned int cnt = 0;
		for(DWORD y = 0; y < desc.Height; y++) {
			for(DWORD x = 0; x < desc.Width; x++) {

				// Fix this code to work with a byte array instead of vector
				DWORD cp = (y * lock.Pitch) + (x * 4);

				DWORD * temp = (DWORD*)(address + (cp / 4));
				DWORD * dest = (DWORD*)(dataBuffer + cnt);
				*dest = *temp >> 24 | *temp << 8;
				cnt++;
			}
		}
		
		// Unlock our texture
		pTexture->UnlockRect(0);

		timeB = m_Timer.GetAbsoluteTime();
		DebugMsg("Test", "Texture Lock Time:  %4.2f", (timeB- timeA));

		timeA = m_Timer.GetAbsoluteTime();
		// Encode the data and have output placed in buffers
		LodePNG_encode((unsigned char**)bytes, (size_t*)len, (BYTE*)dataBuffer, desc.Width, desc.Height, 6, 8);

		timeB = m_Timer.GetAbsoluteTime();
		DebugMsg("Test", "Encoding Time:  %4.2f", (timeB- timeA));
		// Free temporary dataBuffer
		free(dataBuffer);
	}else {
		DebugMsg("ConvertImageInMemoryToPNGBuffer", 
			"Conversion To PNG From Memory Failed. [%X]", retVal);		
	}

	// Release our Texture
	if(pTexture != NULL)
		pTexture->Release();

}

HRESULT ConvertGamerPicToPNGBuffer(DWORD dwPlayerIndex, LPBYTE * fileBuffer, long* fileSize) {

	HRESULT retVal;
	
	DWORD dwDataLen = GAMERPIC_PITCH * GAMERPIC_DIM;
	DWORD * dwDataBuffer = (DWORD*)malloc( dwDataLen );
	DWORD * dwGamerPic = (DWORD*)malloc( dwDataLen );

	if(IsUserSignedIn(dwPlayerIndex)) {
		retVal = XUserReadGamerPicture(dwPlayerIndex, FALSE, (BYTE*)dwGamerPic, GAMERPIC_PITCH, GAMERPIC_DIM, NULL);
	}else{
		retVal = S_FALSE;
	}

	if(retVal == S_OK) {

		// Manipulate Data
		DWORD* pos = (DWORD*)dwDataBuffer;
		for(DWORD y = 0; y < GAMERPIC_DIM; y++) {
			for(DWORD x = 0; x < GAMERPIC_DIM; x++) {

				// Fix this code to work with a byte array instead of vector
				DWORD cp = (y * GAMERPIC_PITCH) + (x * 4);

				DWORD * dwTemp = (DWORD*)(dwGamerPic + (cp / 4));
				*pos = *dwTemp >> 24 | *dwTemp << 8;
				pos++;
			}
		}
		
		// Encode the data and have output placed in buffers
		LodePNG_encode((unsigned char**)fileBuffer, (size_t*)fileSize, (unsigned char*)dwDataBuffer, GAMERPIC_DIM, GAMERPIC_DIM, 6, 8 );
	}

	// Free temporary dataBuffer
	free(dwDataBuffer);
	free(dwGamerPic);
	
	return retVal;
}

long GetGamerPoints(int playerIndex)
{
	if(IsUserSignedIn(playerIndex))
	{
	DWORD requestedSettings[1] ={XPROFILE_GAMERCARD_CRED};
	 
	DWORD cbResults = 0;
	

	// The first call to XUserReadProfileSettings determines the size of the
	// buffer that will be needed to hold the profile information.
	DWORD dwErr = XUserReadProfileSettings(
		0,            // The title ID. Zero for the current title.
		playerIndex,            // The User Index. Zero for the first user.
		1, // The number of settings in the array.
		requestedSettings, // The array of settings to retrieve.
		&cbResults,   // The size (in bytes) of the space needed to retrieve settings.
		0,            // Pointer to results buffer. Set to NULL to retrieve the size needed.
		0);           // Pointer to XOVERLAPPED, or NULL to work synchronously.

	
	// Allocate the buffer using the size returned by the first call to
	// XUserReadProfileSettings.
	XUSER_READ_PROFILE_SETTING_RESULT* pResults =
    (XUSER_READ_PROFILE_SETTING_RESULT*)(new BYTE[cbResults]);

	// Call XUserReadProfileSettings again, using the allocated buffer instead
	// of NULL.
	dwErr = XUserReadProfileSettings(
		0, playerIndex, 1, requestedSettings, &cbResults,
		pResults, // This argument now holds the allocated buffer.
		0);
    XUSER_PROFILE_SETTING* pSrc = pResults->pSettings;
	
	return pSrc->data.nData;
	}
	else
	{
		return 0;
	}
//	_ultoa( pResults[0].pSettings->data.nData, ret, 10 );
//	sprintf_s( ret,100, "%d", pSetting->data.nData  );
              
	//_ultow_s(&pResults->pSettings->data.nData,ret,10);




}
void ExceptionToDebug(struct _EXCEPTION_POINTERS *eps)
{
	char ExcReason[128] = { 0 };
	switch(eps->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.");
			break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.");
			break;
		case EXCEPTION_BREAKPOINT: 
			strcpy_s(ExcReason, sizeof(ExcReason), "A breakpoint was encountered.");
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to read or write data that is misaligned on hardware that does not provide alignment.");
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			strcpy_s(ExcReason, sizeof(ExcReason), "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.");
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to divide a floating-point value by a floating-point divisor of zero.");
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			strcpy_s(ExcReason, sizeof(ExcReason), "The result of a floating-point operation cannot be represented exactly as a decimal fraction.");
			break;
		case EXCEPTION_FLT_INVALID_OPERATION: 
			strcpy_s(ExcReason, sizeof(ExcReason), "This exception represents any floating-point exception not included in this list.");
			break;
		case EXCEPTION_FLT_OVERFLOW: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.");
			break;
		case EXCEPTION_FLT_STACK_CHECK: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The stack overflowed or underflowed as the result of a floating-point operation.");
			break;
		case EXCEPTION_FLT_UNDERFLOW: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.");
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to execute an invalid instruction.");
			break;
		case EXCEPTION_IN_PAGE_ERROR: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to access a page that was not present, and the system was unable to load the page.");// For example, this exception might occur if a network connection is lost while running a program over the network.");
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to divide an integer value by an integer divisor of zero.");
			break;
		case EXCEPTION_INT_OVERFLOW: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The result of an integer operation caused a carry out of the most significant bit of the result.");
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			strcpy_s(ExcReason, sizeof(ExcReason), "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.");
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to continue execution after a non-continuable exception occurred.");
			break;
		case EXCEPTION_PRIV_INSTRUCTION: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.");
			break;
		case EXCEPTION_SINGLE_STEP: 
			strcpy_s(ExcReason, sizeof(ExcReason), "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.");
			break;
		case EXCEPTION_STACK_OVERFLOW: 
			strcpy_s(ExcReason, sizeof(ExcReason), "The thread used up its stack.");
			break;
		default:
			sprintf_s(ExcReason, sizeof(ExcReason), "Unknown reason %08x.", eps->ExceptionRecord->ExceptionCode);
	}

	DebugMsg("Debug", "Exception Error has occured:\n\t%s", ExcReason);
	DebugMsg("Debug", "\tAddress: 0x%08x", eps->ExceptionRecord->ExceptionAddress);
	DebugMsg("Debug", "\tCode   : 0x%08x", eps->ExceptionRecord->ExceptionCode);
	DebugMsg("Debug", "\tFlags  : 0x%08x", eps->ExceptionRecord->ExceptionFlags);
	DebugMsg("Debug", "\tParameter Count: %d", eps->ExceptionRecord->NumberParameters);
	if((eps->ExceptionRecord->NumberParameters > 0) && (eps->ExceptionRecord->NumberParameters < EXCEPTION_MAXIMUM_PARAMETERS))
		for(DWORD x = 0; x < eps->ExceptionRecord->NumberParameters; x++)
			DebugMsg("Debug", "\t\tParameter %02d: 0x%08x", x, eps->ExceptionRecord->ExceptionInformation[x]);

	PCONTEXT pcontx = eps->ContextRecord;
	DWORD contx = pcontx->ContextFlags;
	DebugMsg("Debug", "\tContext Flags  : 0x%08x", contx);
	if(contx & CONTEXT_CONTROL) {
		DebugMsg("Debug", "\tControl Registers:");
		DebugMsg("Debug", "\t\tMSR: %08x IAR: %08x", pcontx->Msr, pcontx->Iar);
		DebugMsg("Debug", "\t\tLR : %08x CTR: %016I64x", pcontx->Lr, pcontx->Ctr);
	}
	if(contx & CONTEXT_INTEGER) {
		DebugMsg("Debug", "\tInteger Registers:");
		DebugMsg("Debug", "\t\tCR : %08x XER: %08x", pcontx->Cr, pcontx->Xer);
		DebugMsg("Debug", "\t\tGPR 0: %016I64x GPR 1: %016I64x GPR 2: %016I64x", pcontx->Gpr0, pcontx->Gpr1, pcontx->Gpr2);
		DebugMsg("Debug", "\t\tGPR 3: %016I64x GPR 4: %016I64x GPR 5: %016I64x", pcontx->Gpr3, pcontx->Gpr4, pcontx->Gpr5);
		DebugMsg("Debug", "\t\tGPR 6: %016I64x GPR 7: %016I64x GPR 8: %016I64x", pcontx->Gpr6, pcontx->Gpr7, pcontx->Gpr8);
		DebugMsg("Debug", "\t\tGPR 9: %016I64x GPR10: %016I64x GPR11: %016I64x", pcontx->Gpr9, pcontx->Gpr10, pcontx->Gpr11);
		DebugMsg("Debug", "\t\tGPR12: %016I64x GPR13: %016I64x GPR14: %016I64x", pcontx->Gpr12, pcontx->Gpr13, pcontx->Gpr14);
		DebugMsg("Debug", "\t\tGPR15: %016I64x GPR16: %016I64x GPR17: %016I64x", pcontx->Gpr15, pcontx->Gpr16, pcontx->Gpr17);
		DebugMsg("Debug", "\t\tGPR18: %016I64x GPR19: %016I64x GPR20: %016I64x", pcontx->Gpr18, pcontx->Gpr19, pcontx->Gpr20);
		DebugMsg("Debug", "\t\tGPR21: %016I64x GPR22: %016I64x GPR23: %016I64x", pcontx->Gpr21, pcontx->Gpr22, pcontx->Gpr23);
		DebugMsg("Debug", "\t\tGPR24: %016I64x GPR25: %016I64x GPR26: %016I64x", pcontx->Gpr24, pcontx->Gpr25, pcontx->Gpr26);
		DebugMsg("Debug", "\t\tGPR27: %016I64x GPR28: %016I64x GPR29: %016I64x", pcontx->Gpr27, pcontx->Gpr28, pcontx->Gpr29);
		DebugMsg("Debug", "\t\tGPR30: %016I64x GPR31: %016I64x", pcontx->Gpr30, pcontx->Gpr31);
	}	

		// Dump our call stack
		DebugMsg("Debug", "\tCall Stack:");
		DWORD stackPtr = (DWORD)pcontx->Gpr1;
		for(DWORD x = 0; x < 25; x++) {
			stackPtr = *(DWORD*)stackPtr;
			DWORD lr = *((DWORD*)(stackPtr - 0x08));
			DebugMsg("Debug", "\t\t0x%08X", lr);
		}

}

LONG WINAPI UnHandleExceptionFilter(struct _EXCEPTION_POINTERS
*lpExceptionInfo)
{
	DebugMsg("Debug","Unhandled exception occured...");
	ExceptionToDebug(lpExceptionInfo);

	SYSTEMTIME T;
	GetLocalTime(&T);
	string NewLogPath = sprintfa("%scrash.%d%02d%02d%02d%02d%02d.log", LOG_PATH, T.wYear, T.wMonth, T.wDay, T.wHour, T.wMinute, T.wSecond);
	rename(sprintfa("%s%s", LOG_PATH, LOG_FILE).c_str(), NewLogPath.c_str());

	XOVERLAPPED overlapped;
	MESSAGEBOX_RESULT mbresult;
	HANDLE hEvent = WSACreateEvent();
	memset(&overlapped,0,sizeof(overlapped));
	overlapped.hEvent = hEvent;
	memset(&mbresult,0,sizeof(mbresult));
	LPCWSTR btnOptions[3] = {L"Restart FSD", L"Go back to NXE",L"Try to continue..."};
	XShowMessageBoxUI(XUSER_INDEX_ANY,L"FSD CRASH",L"Freestyle Dash crashed, what would you like to do?",3,btnOptions,0,XMB_ERRORICON,&mbresult,&overlapped);
	WaitForSingleObject( hEvent, INFINITE );
	
	
	WSAResetEvent( hEvent);
	
	if(mbresult.dwButtonPressed == 0)
	{
		DebugMsg("Debug","Restart pressed");
		Restart();
	}
	else
	{
		if(mbresult.dwButtonPressed == 1)
		{
			BackToNXE();
		}
		else
		{
			if(mbresult.dwButtonPressed == 2)
			{

			}
			else
			{
				DebugMsg("Debug","Nothing choosed, restart");
				BackToNXE();
			}
		}
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}


int GetGamerPicture(int playerIndex,PBYTE ret,BOOL small,DWORD pitch,DWORD height)
{
if(IsUserSignedIn(playerIndex))
	{
	DWORD dwErr = XUserReadGamerPicture(playerIndex,small,ret,pitch,height,NULL);
	
	return dwErr;
}
else
{
	return -1;
}
}
SIZE_T GetAvailableRam()
{
	MEMORYSTATUS stat;
	GlobalMemoryStatus( &stat );
	
	return stat.dwAvailPhys;	
}
bool IsUserSignedIn(int playerIndex)
{
	  XUSER_SIGNIN_STATE State = XUserGetSigninState( playerIndex );
		if( State != eXUserSigninState_NotSignedIn )
        {
			return true;
		}
		else
		{
			return false;
		}
}
int GetGamerTag(int playerIndex,char *ret)
{
	if(IsUserSignedIn(playerIndex))
	{
	DWORD dwErr = XUserGetName(playerIndex,ret,100);
	
	return dwErr;
	}
	else
	{
		return -1;
	}
/*	LPSTR szUserName;
	
	return 0;*/
}

string GetGamerId(int playerIndex) {
	string gamerId = "";
	XUID xuid;

	HRESULT hr = XUserGetXUID(playerIndex, &xuid);
	if(hr == ERROR_SUCCESS) {
		char id[17];
		sprintf_s(id, 17, "%I64X", xuid);
		gamerId = id;	
	}
	return gamerId;
}

string GetLocalTimeAsI64String() {

    SYSTEMTIME LocalSysTime;
    GetLocalTime( &LocalSysTime );
    FILETIME LocalFileTime;
    SystemTimeToFileTime( &LocalSysTime, &LocalFileTime );

	ULARGE_INTEGER t;
	t.HighPart = LocalFileTime.dwHighDateTime; 
	t.LowPart = LocalFileTime.dwLowDateTime;
	char n[17];
	sprintf_s(n, 17, "%I64X", t.QuadPart);
	return n;
}


 // This function converts the 32bit Unix time structure to the FILETIME 
// structure. 
// The time_t is a 32-bit value for the number of seconds since January 1, 
// 1970. A FILETIME is a 64-bit for the number of 100-nanosecond periods 
// since January 1, 1601. Convert by multiplying the time_t value by 1e+7 
// to get to the same base granularity, then add the numeric equivalent 
// of January 1, 1970 as FILETIME. 
void UnixTimeToFileTime(time_t* pt, LPFILETIME pft) 
{ 
        LONGLONG ll; // 64 bit value 
        ll = Int32x32To64(*pt, 10000000) + 116444736000000000ui64; 
        pft->dwLowDateTime = (DWORD)ll; 
        pft->dwHighDateTime = (DWORD)(ll >> 32); 
} 

// This function converts the FILETIME structure to the 32 bit 
// Unix time structure. 
// The time_t is a 32-bit value for the number of seconds since 
// January 1, 1970. A FILETIME is a 64-bit for the number of 
// 100-nanosecond periods since January 1, 1601. Convert by 
// subtracting the number of 100-nanosecond period betwee 01-01-1970 
// and 01-01-1601, from time_t the divide by 1e+7 to get to the same 
// base granularity. 
void FileTimeToUnixTime(LPFILETIME pft, time_t* pt) 
{ 
        LONGLONG ll; // 64 bit value 
        ll = (((LONGLONG)(pft->dwHighDateTime)) << 32) + pft->dwLowDateTime; 
        *pt = (time_t)((ll - 116444736000000000ui64)/10000000ui64); 
} 

// This function converts the 32 bit Unix time structure to 
// the SYSTEMTIME structure 
void UnixTimeToSystemTime(time_t* pt, LPSYSTEMTIME pst) 
{ 
        FILETIME ft; 
        UnixTimeToFileTime(pt, &ft); 
        FileTimeToSystemTime(&ft, pst); 
} 

// This function coverts the SYSTEMTIME structure to 
// the 32 bit Unix time structure 
void SystemTimeToUnixTime(LPSYSTEMTIME pst, time_t* pt) 
{ 
        FILETIME ft; 
        SystemTimeToFileTime(pst, &ft); 
        FileTimeToUnixTime(&ft, pt); 
} 

SYSTEMTIME TimeI64StringToSystemTime(string t) 
{
	string time = "0x";
	time.append(t);

	ULARGE_INTEGER li;
	sscanf_s(time.c_str(), "%I64X", &li.QuadPart );

    FILETIME LocalFileTime;
	LocalFileTime.dwHighDateTime = li.HighPart;
	LocalFileTime.dwLowDateTime = li.LowPart;

	SYSTEMTIME LocalSysTime;
	FileTimeToSystemTime(&LocalFileTime, &LocalSysTime);

	return LocalSysTime;
}

wstring SystemTimeToLocalString(const SYSTEMTIME& stime)
{
	// THIS SHOULD ABSOLUTELY PAY ATTENTION TO TIME AND DATE FORMAT, BUT IT DOESN'T
	FILETIME ft;
	SystemTimeToFileTime(&stime, &ft);
	FILETIME lt;
	FileTimeToLocalFileTime(&ft, &lt);
	SYSTEMTIME nst;
	FileTimeToSystemTime(&lt, &nst);

	string mer = "AM";
	if (nst.wHour > 12) {
		nst.wHour -= 12;
		mer = "PM";
	}
	ostringstream os;
	os << nst.wYear << "-" << nst.wMonth << "-" << nst.wDay << " ";
	os << nst.wHour << ":" << setw(2) << std::setfill('0') << nst.wMinute << ":" << setw(2) << std::setfill('0') << nst.wSecond << " " << mer; 
	return strtowstr(os.str());
}

/* MATTIE: replaced by XeDebug
void DebugMsg(const char *format, ...) 
{
	char debugbuf[4096];

	va_list ap;
	va_start (ap, format);
	vsprintf_s(debugbuf,4096, format, ap);
	va_end (ap);

	debugLog(debugbuf);
}
*/

void FTPMsg(const char *format, ...) 
{
	char debugbuf[4096];

	va_list ap;
	va_start (ap, format);
	vsprintf_s(debugbuf,4096, format, ap);
	va_end (ap);
	DebugMsg("FTP", "%s",debugbuf);
//	ftpLog(debugbuf);
}


void debugLog(const char* output)
{
    ofstream writeLog;

    writeLog.open("game:\\debug.log",ofstream::app);
    if (writeLog.is_open())
    {
      writeLog.write(output,strlen(output));
      writeLog.write("\n",1);
    }
    writeLog.close();
}

void ftpLog(const char* output)
{
    ofstream writeLog;

    writeLog.open("game:\\ftp.log",ofstream::app);
    if (writeLog.is_open())
    {
      writeLog.write(output,strlen(output));
      writeLog.write("\n",1);
    }
    writeLog.close();
}

// returns 0 on success, creates symbolic link on it's own to the new szDrive
HRESULT mountCon(CHAR* szDrive, const CHAR* szDevice, const CHAR* szPath) {

   CHAR szMountPath[MAX_PATH];
   sprintf_s(szMountPath, MAX_PATH, "\\??\\%s\\%s", szDevice, szPath);
   return XamContentOpenFile(0xFE, szDrive, szMountPath, 0x4000043, 0, 0, 0);
}

// returns 0 on success, destorys symbolic link as well
HRESULT unmountCon(CHAR* szDrive, bool bSaveChanges) {

   CHAR szMountPath[MAX_PATH];
   sprintf_s(szMountPath, MAX_PATH, "\\??\\%s", szDrive);

   if(bSaveChanges)
		XamContentFlush(szMountPath, 0);

   return XamContentClose(szMountPath, 0);
}

bool InitNetwork()
{
	DWORD dwStatus = XNetGetEthernetLinkStatus();

	int m_bIsOnline = ( dwStatus & XNET_ETHERNET_LINK_ACTIVE ) != 0;

	if( !m_bIsOnline )
	{
		printf("NO ETHERNET LINK ACTIVE\n");
		return false;
	}

	XNetStartupParams xnsp;
	memset(&xnsp, 0, sizeof(xnsp));
	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;




xnsp.cfgSockDefaultRecvBufsizeInK = 32; // default = 16 
xnsp.cfgSockDefaultSendBufsizeInK = 32; // default = 16 

	INT iResult = XNetStartup( &xnsp );

	if( iResult != NO_ERROR )
	{
		DebugMsg("InitNetwork","XNETSTARTUP ERROR");
		return false;
	}

	WSADATA WsaData;
	iResult = WSAStartup( MAKEWORD( 2, 2 ), &WsaData );
	if( iResult != NO_ERROR )
	{
		DebugMsg("InitNetwork","WSASTARTUP ERROR");
		return false;
	}

	string m_IP = "N/A";
	XNADDR xnaddr;
	IN_ADDR ipaddr, lipaddr;
	XNetGetTitleXnAddr( &xnaddr );
	ipaddr=xnaddr.ina;
	char szip[16];
	if (memcmp(&ipaddr, &lipaddr, sizeof(ipaddr))!=0)
	{
		XNetInAddrToString( xnaddr.ina, szip, 16);
		m_IP = sprintfaA("%s", szip);
	}

	if(strcmp(m_IP.c_str(), "N/A") == 0)
		return false;

	if(strcmp(m_IP.c_str(), "0.0.0.0") == 0)
		return false;

	DebugMsg("InitNetwork","Network initialized");
	return true;
}

void SendXuiKeyStroke(WORD VirtualKey)
{
	XINPUT_KEYSTROKE keyStroke;

	keyStroke.Flags = XINPUT_KEYSTROKE_KEYDOWN;
	keyStroke.UserIndex = XUSER_INDEX_ANY;
	keyStroke.VirtualKey = VirtualKey;
	keyStroke.HidCode = 0;
	keyStroke.Unicode = 0;

	XuiProcessInput( &keyStroke );
}

void RecursiveFolderSearch(vector<string> * m_Paths, string source, string filetype)
{
	FileBrowser fSource;
	fSource.CD(source);

	string sourceFolder = fSource.GetCurrentPath() + "\\";

	//Catch any "\\" in the path
	sourceFolder = str_replaceallA(sourceFolder,"\\\\","\\");

	vector<string> folders = fSource.GetFolderList();

	for(unsigned int x=0;x<folders.size();x++)
	{
		if(strcmp(folders.at(x).c_str(),"..") != 0)
		{
			string sourceFile = sourceFolder + folders.at(x);
			RecursiveFolderSearch(m_Paths, sourceFile, filetype);
		}
	}
	RecursiveFileSearch(m_Paths, fSource.GetCurrentPath(), filetype);
}

void RecursiveFileSearch(vector<string> * m_Paths, string source, string filetype)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd, result;

	searchcmd = source + "\\" + "*." + filetype;

	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {

			result = source + "\\" + findFileData.cFileName;

			m_Paths->push_back(result);

		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
}

HRESULT GetKernelVersion(XBOX_KRNL_VERSION * kernelVer)
{
	DWORD kernel = XamUpdateGetCurrentSystemVersion();

	kernelVer->Major = kernel >> 28;
	kernelVer->Minor = (kernel >> 24) & 0xF;
	kernelVer->Build = (kernel >> 8) & 0xFFFF;
	kernelVer->Qfe = kernel & 0xFF;

	return S_OK;
}
