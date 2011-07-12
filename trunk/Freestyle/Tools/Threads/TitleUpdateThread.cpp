#include "stdafx.h"

#include "TitleUpdateThread.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"
#include "../../Tools/SQLite/FSDSql.h"


bool TitleUpdateThread::CreateTitleUpdateThread( void )
{
	bool bResult = CreateThread( CPU2_THREAD_1 );

	return bResult;
}

void TitleUpdateThread::cancel()
{
	DebugMsg("TitleUpdateThread","Cancelling");
	//loop = false;
	//Terminate();
}


unsigned long TitleUpdateThread::Process (void* parameter)
{
	DebugMsg("TitleUpdateThread", "Starting process");
	vector<TitleUpdatePath *> drive = FSDSql::getInstance().getTitleUpdatePaths();
	string sourcedrive;
	string destdrive;
	for (unsigned int x = 0; x < drive.size(); x++)
	{
		if (!drive.at(x)->isBackup)
		{
			sourcedrive = drive.at(x)->path;
		}
		bool Found = false;
		for (unsigned int y = 0; y < drive.size(); y++)
		{
			if (drive.at(y)->isBackup)
			{
				if (drive.at(y)->backupFor == drive.at(x)->pathId)
				{
					destdrive = drive.at(y)->path;
					Found = true;
				}
			}
		}
		if (!Found)
		{
			destdrive = sprintfaA("%sTitleUpdates\\%s", SETTINGS::getInstance().getDataPath().c_str(), drive.at(x)->deviceID.c_str());
			RecursiveMkdir(destdrive);
		}

		WIN32_FIND_DATA findFileData;
		memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
		string searchcmd;

		searchcmd = sprintfaA("%s\\*", sourcedrive.c_str());
		searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
		DebugMsg("TitleUpdateThread","Looking for cache files in %s",searchcmd.c_str());

		HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				string destpath = sprintfaA("%s\\%s", destdrive.c_str(), findFileData.cFileName);
				string sourcepath = sprintfaA("%s\\%s", sourcedrive.c_str(), findFileData.cFileName);
				if (!FileExists(destpath) && destpath.find("TU") != string::npos)
				{
					FileOperationManager::getInstance().AddFileOperation(sourcepath, destpath, false);
				}
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}

		int pos = sourcedrive.find(":");
		sourcedrive = sourcedrive.substr(0, pos);
		string destinationdrive = sprintfaA("%s:\\cache", sourcedrive.c_str());
		sourcedrive = sprintfaA("%s:\\content\\0000000000000000", sourcedrive.c_str());
		searchcmd = sprintfaA("%s\\*", sourcedrive.c_str());
		searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
		
	
		hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					TitleIDScan(sourcedrive + "\\" + findFileData.cFileName, destinationdrive );
				}
			} while (FindNextFile(hFind, &findFileData));
			FindClose(hFind);
		}
	}

	FileOperationManager::getInstance().DoWork();

	return 0;
}

void TitleUpdateThread::TitleIDScan(string path, string destdrive){

	DebugMsg("TitleUpdateThread", "Scanning path: %s, move to : %s", path.c_str(), destdrive.c_str());
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd;
	searchcmd = sprintfaA("%s\\*", path.c_str());
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");

	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (strcmp(findFileData.cFileName, "000B0000")==0)
					FileScan(path + "\\" + findFileData.cFileName, destdrive);
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
} 

void TitleUpdateThread::FileScan(string path, string destdrive) {

	DebugMsg("TitleUpdateThread", "Scanning path: %s, move to : %s", path.c_str(), destdrive.c_str());
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd;
	searchcmd = sprintfaA("%s\\*", path.c_str());
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");

	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			DebugMsg("TitleUpdateThread", "Foudn %s in %s", findFileData.cFileName, path.c_str());
			string file = findFileData.cFileName;
			DebugMsg("TitleUpdateThread", "Found %s", file.c_str());
			if (file.find("TU") != string::npos || file.find("tu") != string::npos)
			{
				string destpath = sprintfaA("%s\\%s", destdrive.c_str(), findFileData.cFileName);
				string sourcepath = sprintfaA("%s\\%s", path.c_str(), findFileData.cFileName);
				DebugMsg("TitleUpdateThread", "Copy %s to %s", sourcepath.c_str(), destpath.c_str());
				if (!FileExists(destpath))
				{
					FileOperationManager::getInstance().AddFileOperation(sourcepath, destpath, false);
				}
			}
		} while (FindNextFile(hFind, &findFileData));
	}
	FindClose(hFind);
}
