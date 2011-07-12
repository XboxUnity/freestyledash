#include "stdafx.h"

#include "CopyDVDThread.h"
#include "../settings/Settings.h"
#include "../../Tools/XEX/XboxExecutable.h"
#include "../../Tools/XEX/XeXtractor.h"
#include "../../Tools/XEX/SPA.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"


bool CopyDVDThread::CreateCopyDVDThread( void )
{
	bool bResult = CreateThread( CPU2_THREAD_1 );

	return bResult;
}

void CopyDVDThread::cancel()
{
	DebugMsg("CopyDVDThread","Cancelling");
	//loop = false;
	//Terminate();
	XamSetDvdSpindleSpeed(1);
}

unsigned long CopyDVDThread::Process (void* parameter)
{
	DebugMsg("CopyDVDThread", "Starting process");
	loop = true;
	while (loop)
	{
		if (DvdCopyProcess == 1)
		{
			if (gameType == 1)
			{
				DvdCopyProcess = 0;
				gameType = 0;
				DebugMsg("CopyDVDThread", "Starting 360 extraction");
				string temptitle = getDVDTitleXeX();
				Title = temptitle;

			} else if (gameType == 2)
			{
				DvdCopyProcess = 0;
				gameType = 0;
				DebugMsg("CopyDVDThread", "Starting original extraction");
				string temptitle = getDVDTitleXBE();
				Title = temptitle;
			}
		} else if (DvdCopyProcess == 2)
		{
			XamSetDvdSpindleSpeed(DVD_SPEED_12X);
			DvdCopyProcess = 0;
			WIN32_FIND_DATA findFileData;
			memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
			string searchcmd = "dvd:\\*";
			searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
			HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do {
					if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (strcmp(findFileData.cFileName, "$SystemUpdate") != 0)
						{
							string source = sprintfaA("dvd:\\%s", findFileData.cFileName);
							string dest = sprintfaA("%s\\%s", fullcopy.c_str(), findFileData.cFileName);
							if (!FileExists(dest))
							{	
								_mkdir(dest.c_str());
							}
							currentItem = findFileData.cFileName;
							FileOperationManager::getInstance().AddFolderOperation(source,dest,false);
						} else if (strcmp(findFileData.cFileName, "$SystemUpdate") == 0 && IncludeUpdate)
						{
							string source = sprintfaA("dvd:\\%s", findFileData.cFileName);
							string dest = sprintfaA("%s\\%s", fullcopy.c_str(), findFileData.cFileName);
							if (!FileExists(dest))
							{	
								_mkdir(dest.c_str());
							}
							currentItem = findFileData.cFileName;
							FileOperationManager::getInstance().AddFolderOperation(source,dest,false);
						}
					} else {
						string source = sprintfaA("dvd:\\%s", findFileData.cFileName);
						string dest = sprintfaA("%s\\%s", fullcopy.c_str(), findFileData.cFileName);
						FileOperationManager::getInstance().AddFileOperation(source, dest, false);
						currentItem = findFileData.cFileName;
					}
				} while (FindNextFile(hFind, &findFileData));
			}FindClose(hFind);
			currentItem = "Complete";
			FileOperationManager::getInstance().DoWork();
		}
		Sleep(100);
	}
	
	return 0;
}

string CopyDVDThread::ReturnTitle()
{
	return Title;
}

void CopyDVDThread::getTitle(int type)
{
	gameType = type;
	Title = "Extracting";
	DvdCopyProcess = 1;
}

void CopyDVDThread::startScan(bool Update, string path)
{
	IncludeUpdate = Update;
	fullcopy = path;
	currentItem = "";
	DvdCopyProcess = 2;
}

string CopyDVDThread::ReturnCurrentItem()
{
	return currentItem;
}

string CopyDVDThread::getDVDTitleXBE()
{
	string copydvdname;
	int namelength;
	XboxExecutable xbe;
	xbe.OpenExecutable("dvd:\\default.xbe");
	xbe.GetTitleName(&copydvdname, &namelength);
	return copydvdname;
}

string CopyDVDThread::getDVDTitleXeX()
{
	// Open the XEX
	XeXtractor xeXtractor;
	xeXtractor.OpenXex("dvd:\\default.xex");

	// Get Status of XEX
	BOOL isRetail = xeXtractor.IsRetailXex();
	DebugMsg("CopyDVDThread", "XEX TYPE:  %s", isRetail ? "RETAIL" : "DEVKIT");

	// Get our resource
	PBYTE spaData;
	DWORD spaSize = 0;
	HRESULT hr = xeXtractor.ExtractTitleSPA(&spaData, &spaSize);
	if(hr != S_OK) {
		xeXtractor.CloseXex();
		DebugMsg("CopyDVDThread", "ExtractTitleSPA function failed.  Error:  %x", hr);
		
		//delete [] spaData;
		
		return "Invalid Disc";
	}	

	// Verify that the SPA data is valid
	if(*((int*)spaData) != 0x58444246 )
	{
		DebugMsg("CopyDVDThread", "Magic Invalid- trying different encryption");
		
		// Flip our machine type to try the other
		xeXtractor.isRetailXex = !xeXtractor.isRetailXex;
		
		// Get our resource
		HRESULT hr = xeXtractor.ExtractTitleSPA(&spaData, &spaSize);
		if(hr != S_OK) {
			xeXtractor.CloseXex();
			DebugMsg("CopyDVDThread", "ExtractTitleSPA function failed.  Error:  %x", hr);
			delete [] spaData;
			return "Invalid Disc";				
		}

		// Verify that the SPA data is valid
		if(*((int*)spaData) != 0x58444246 )
		{
			xeXtractor.CloseXex();
			DebugMsg("CopyDVDThread", "Unknown Error - Invalid SPA File Created");
			delete [] spaData;
			return "Invalid Disc";		
		}
	}

	// Close our xex
	xeXtractor.CloseXex();

	string resPath = "game:\\CopyDVD.spa";
	// Found Good Spa File - Write out our resource
	FILE* output;
	fopen_s(&output,resPath.c_str(), "wb+");
	fwrite(spaData, spaSize, 1, output);
	fclose(output);
	delete [] spaData;

	// If resource exists; extract the title name and save it as title.txt
	if(FileExists(resPath))
	{
		HRESULT retVal = NULL;
		DebugMsg("CopyDVDThread", "Resource Path:  %s", resPath.c_str());

		SPA spa;
		retVal = spa.OpenSPA(resPath);
		if(retVal != 1)
		{
			DebugMsg("CopyDVDThread", "Failed to open Resource.spa");
			spa.CloseSPA();
			return "Invalid Disc";
		}
		string copydvdname;
		retVal = spa.GetTitleName(&copydvdname, LOCALE_ENGLISH);
		if(retVal != 1){
			DebugMsg("CopyDVDThread", "Could not extract title name: %x", retVal);	
			spa.CloseSPA();
			return "Invalid Disc";
		}
		DebugMsg("CopyDVDThread", "Extracted Title:  %s", copydvdname.c_str());
		
		spa.CloseSPA();
		return copydvdname;
	}
	return "Invalid Disc";
}