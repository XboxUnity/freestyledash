#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Settings/Settings.h"
#include "../../Managers/Skin/SkinManager.h"
#include "../../FileBrowser/FileBrowser.h"
//#include "../../FSDMessages/FSDMessages.h"
#include "../../XEX/Xbox360Container.h"
#include "../../XFSD/XFSD.h"
#include "../../Profile/Account/Account.h"
#include "../../GameContent/QueueThreads/FileQueue.h"
#include "../../GameContent/QueueThreads/FileTexQueue.h"
#include "../../GameContent/QueueThreads/TextureQueue.h"
#include "../../GameContent/QueueThreads/ReleaseQueue.h"
#include "../../Generic/PNG/lodepng.h"
#include "../../Texture/TextureCache.h"
#include "../../ContentList/ContentItemNew.h"
#include "../../ContentList/ContentManager.h"
#include "../../SQLite/FSDSql.h"

class BasicTestCommand:public DebugCommand
{
public :

	BasicTestCommand()
	{
		m_CommandName = "BasicTest";
	}

	void Perform(string parameters)
	{
		ContentManager::getInstance().CheckCovers();
		return;
		
		/*SYSTEMTIME st;
		FILETIME ft;
		LARGE_INTEGER li;
		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &ft);
		li.LowPart = ft.dwLowDateTime;
		li.HighPart = ft.dwHighDateTime;

		DebugMsg("BasieTest", "Test = %016I64d", li);

		*/
		/*if(parameters == "bits")
		{
			DWORD flags = 0;
			flags |= 0x20; flags |= 80;
			DebugMsg("Current Flags", "Current Flags:  0x%08X", flags);

			SETMAXSS(flags, 20);
			DebugMsg("Current Flags", "Current Flags:  0x%08X", flags);

		}
		if(parameters == "png")
		{
			string path = "hdd1:\\box.dds";
		}
		if(parameters == "PNG")
		{

			DebugMsg("Test", "Test");
			string path = "hdd1:\\box.dds";

			FILE *fHandle;
			fopen_s(&fHandle, path.c_str(), "rb");
			BYTE * buffer;
			DWORD size;

			// Get file length 
			fseek(fHandle, 0, SEEK_END); 
			size = ftell(fHandle); 
			fseek(fHandle, 0, SEEK_SET); 

			DDSHEADER temp;

			DWORD newSize = size - sizeof(DDSHEADER);
			buffer = (BYTE*)malloc(newSize);

			fread((void*)&temp, sizeof(DDSHEADER), 1, fHandle);
			fread(buffer, size - sizeof(DDSHEADER), 1, fHandle);
			fclose(fHandle);

			vector<unsigned char> in;
			for(int i = 0; i < (size-sizeof(DDSHEADER)); i++)
			{
				in.push_back((unsigned char)buffer[i]);
			}

			//vector<unsigned char> out;

			
			//LodePNG::encode(out, in, (unsigned int)temp.ddsd.dwWidth, (unsigned int)temp.ddsd.dwHeight);
			string fileOut = "hdd1:\\image.png";
			LodePNG::encode(fileOut, in, 10, 10, 6, 8);
			//LodePNG::saveFile(out, fileOut);
			//LodePNG::saveFile((out), fileOut);
			//IDirect3DTexture9 * pTexture;
			//TextureCache::LoadD3DTextureFromFile(path, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, &pTexture);
			
			

			//LoadPNG pngFile;

			

		}

		// Dump memory leaks
		if(parameters == "leaks")
			_CrtDumpMemoryLeaks();

		if(parameters == "File") {
			FileQueue::getInstance().DebugListFileQueue();
		}
		else if(parameters == "FileTex") {
			FileTexQueue::getInstance().DebugListFileTexQueue();
		}
		else if(parameters == "Texture") {
			TextureQueue::getInstance().DebugListTextureQueue();
		}
		else
			ReleaseQueue::getInstance().DebugListReleaseQueue();
//		char *pUserInfo = (char*)0x81BF5DC0;
//		FILE * file;
//		fopen_s(&file, "hdd1:\\userinfo.bin", "wb");
//		fwrite(pUserInfo, 0x1A0, 1, file);
//		fclose(file);
		*/
		//PatchInJump((UINT32*)0x9140291C, (UINT32)XamUserLogonHook, FALSE);
		//DebugMsg("BasicTest", "XamUserLogon Function Patched");
	}

	struct ProfileInfo
	{
		string gamerTag;
		string profileXUID;
		string profilePath;
		bool isOnline;
	};


	void ListProfiles(string parameters)
	{
		vector<string> m_ProfileFolders;
		vector<ProfileInfo> m_Paths;
		FileBrowser fb;


		DebugMsg("TesT", "Size of short = %d", sizeof(short));
		DebugMsg("TesT", "Size of int = %d", sizeof(int));
		DebugMsg("TesT", "Size of Long = %d", sizeof(long));
		string basePath = "hdd1:\\Content\\";
		fb.CD(basePath);
		m_ProfileFolders = fb.GetFolderList();
		
		int nSize = m_ProfileFolders.size();

		for(int i = 0; i < nSize; i++)
		{
			if(strcmp(m_ProfileFolders.at(i).c_str(), "0000000000000000") != 0)
			{
				if(strcmp(m_ProfileFolders.at(i).c_str(), "..") != 0)
				{
					ProfileInfo prof;
					prof.profilePath = basePath + m_ProfileFolders.at(i);
					prof.profileXUID = m_ProfileFolders.at(i);
					prof.isOnline = IsXUIDSignedIn(prof.profileXUID);
					prof.gamerTag = "";
					m_Paths.push_back(prof);
				}
			}
		}

		Xbox360Container conFile;
		nSize = m_Paths.size();
		for(int i = 0; i < nSize; i++)
		{
			string fullPath = m_Paths.at(i).profilePath + "\\FFFE07D1\\00010000\\" + m_Paths.at(i).profileXUID;
			//bool isOnline = IsXUIDSignedIn(m_Paths.at(i));
			if(m_Paths.at(i).isOnline == false && FileExistsA(fullPath))
			{
				DebugMsg("BasicTest", "Account File Found:  %s", fullPath.c_str());
				conFile.OpenContainer(fullPath);
				conFile.Mount("con");
				if(FileExists("con:\\Account"))
				{
					FILE * fHandle;
					fopen_s(&fHandle, "con:\\Account", "rb");
					char * buffer;
					buffer = (char*)malloc(0x194);
					fread(buffer, 0x194, 1, fHandle);
					fclose(fHandle);

					int newSize = 0x17C;
					XAMACCOUNTINFO account;
				
					XeKeysUnObfuscate(1, buffer, 0x194, (char*)&account, &newSize);

					//string GamerTag = wstrtostr(account.GamerTag);
					//m_Paths.at(i).gamerTag = GamerTag;
					//DebugMsg("BasicTest", "Folder:  %s (%s) - GamerTag:  %s", fullPath.c_str(), m_Paths.at(i).isOnline == true ? "online" : "offline",m_Paths.at(i).gamerTag.c_str());

					//free(buffer);
					//free(newBuffer);
				}
				conFile.UnMount();
			}
		}

	}

	bool IsXUIDSignedIn(string sXUID)
	{
		bool isOnline = false;

		HRESULT hr = NULL;
		for(int i = 0; i < 4; i++)
		{
			XUID xuid;
			hr = XUserGetXUID(i, &xuid);
			if(hr == S_OK)
			{
				string szXUID = sprintfaA("%I64X", xuid);
				//DebugMsg("BasicTest", "Checked XUID:  %s,  Input XUID:  %s", szXUID.c_str(), sXUID.c_str());
				if(strcmp(sXUID.c_str(), szXUID.c_str()) == 0){
					isOnline = true;
					break;
				}
			}
		}
		return isOnline;
	}

};