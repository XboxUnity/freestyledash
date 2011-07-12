#include "stdafx.h"

#include "TitleUpdateManager.h"
#include "../../SQLite/FSDSql.h"
#include "../../Managers/FileOperation/FileOperationManager.h"
#include "../../XEX/Xbox360Container.h"
#include "../../SQLite/FSDSql.h"
#include "../../Generic/Tools.h"
#include "../../Managers/Drives/DrivesManager.h"


bool TitleUpdateManager::CreateTitleUpdateManager( void )
{
	bool bResult = CreateThread( CPU2_THREAD_1 );
	pause = true;
	Function = 0;
	szStatus="";
	return bResult;
}


void TitleUpdateManager::scan() {
	pause = false;
	working = true;
	Function = 1;
}

void TitleUpdateManager::cancel()
{
	DebugMsg("TitleUpdateManager","Cancelling");
	Function = 0;
	working = false;
	pause = true;
	//Terminate();
}

string TitleUpdateManager::getStatus() { return szStatus; }

void TitleUpdateManager::DeleteTitleUpdates(DWORD TitleID, SQLTitleUpdate* TitleUpdate) {
	pause = false;
	working = true;
	Function = 3;
	TID = TitleID;
	TU = TitleUpdate;
}

void TitleUpdateManager::ConsolidateTitleUpdates(DWORD TitleID) {
	pause = false;
	working = true;
	Function = 2;
	TID = TitleID;
}


unsigned long TitleUpdateManager::Process (void* parameter)
{
	for(;;)
	{
		if (pause) {
			//DebugMsg("PROCESS", "WE ARE IN THE PAUSE LOOP, OHH NOEEZZ");
			Sleep(1000);
		}
		else if (Function == 1)
		{
			FSDSql::getInstance().cleanBadTUPaths();
			vector<Drive* const> devices;
			DrivesManager::getInstance().getMountedDrives(&devices);
			for(unsigned int i = 0; i < devices.size(); i++) {
				string mp = devices[i]->getMountPoint();

				// the only places where TUs can live
				if (mp.compare("Hdd1:") == 0 ||
					mp.compare("Memunit0:") == 0 ||
					mp.compare("Memunit1:") == 0 ||
					mp.compare("OnBoardMU:") == 0 ||
					mp.compare("USBMUCache0:") == 0 ||
					mp.compare("USBMUCache1:") == 0 ||
					mp.compare("USBMUCache2:") == 0) {
						string cachePath = "\\cache";
						string contentPath = "\\content\\0000000000000000";
						string dataPath = sprintfaA("game:\\data\\TitleUpdates\\%s",devices[i]->getSerialStr().c_str());
						FSDSql::getInstance().addTitleUpdatePath(devices[i]->getSerialStr(), dataPath, cachePath, contentPath);
				}
			}
			
			vector<TitleUpdatePath*> TUPaths = FSDSql::getInstance().getTitleUpdatePaths();
			for (unsigned int i =0; i < TUPaths.size(); i++) {
				
				ScanCache(TUPaths.at(i));
				ScanContent(TUPaths.at(i));

				string path = TUPaths.at(i)->dataPath;
				DebugMsg("TitleUpdateManager", "Looking for TitleUpdates in [%s] [%d]", path.c_str(), TUPaths.at(i)->pathId);
				szStatus = path;
				if (path.c_str()[path.length() -1] != '\\')
				{
					path = path + "\\";
				}
				WIN32_FIND_DATA FindFileData;
				memset(&FindFileData,0,sizeof(WIN32_FIND_DATA));
				string SearchCmd = path + "*";
				string FullFolder = "";
				bool FileFound = true;
				HANDLE hFind = NULL;
				for (hFind = FindFirstFile(SearchCmd.c_str(), &FindFileData); (FileFound && (hFind != INVALID_HANDLE_VALUE)); FileFound = FindNextFile(hFind, &FindFileData))
				{
					szStatus = FindFileData.cFileName;
					if (Function == 2 || Function ==3)
					{
						break;
					} else if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						newTuFolderScan(TUPaths.at(i), FindFileData.cFileName);
					} else {
						string conFullPath = path + FindFileData.cFileName;
						Xbox360Container conFile;
						HRESULT hr = conFile.OpenContainer(conFullPath);
						if(hr != S_OK) {
							conFile.CloseContainer();
							continue;
						}

						// Determine what type of content this is
						DWORD contentType = 0x0;
						if((hr = conFile.GetContentType(&contentType)) == S_OK){
							if(contentType == 0xB0000) {
								DWORD TmpId;
								hr = conFile.GetTitleId(&TmpId);
								if (hr == S_OK)
								{
									DebugMsg("TitleUpdateManager", "Found TitleUpdate for TitleID [%s] [%08X]",wstrtostr(conFile.GetDisplayName()).c_str(), TmpId);
									ULONGLONG TUID = FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), FindFileData.cFileName, "", 0, -1, TUPaths.at(i)->pathId, 0 );
									string file = TUPaths.at(i)->cachePath + "\\" + FindFileData.cFileName;
									if (FileExists(file))
									{
										Xbox360Container conCheck;
										HRESULT hr1 = conCheck.OpenContainer(file);
										if (hr != S_OK) {
											conCheck.CloseContainer();
											continue;
										}
										DWORD contentCheck = 0x0;
										if ((hr1 = conCheck.GetContentType(&contentCheck)) == S_OK){
											if (contentCheck == 0xB0000) {
												if (strcmp(wstrtostr(conFile.GetDisplayName()).c_str(), wstrtostr(conCheck.GetDisplayName()).c_str()) == 0)
												{
													FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), FindFileData.cFileName, "", 1, TUID, TUPaths.at(i)->pathId, 1 );
												}
											}
										}
										conCheck.CloseContainer();
									}
									file = sprintfaA("%s\\%08X\\000B0000\\%s", TUPaths.at(i)->contentPath.c_str(), TmpId, FindFileData.cFileName);
									if (FileExists(file))
									{
										Xbox360Container conCheck;
										HRESULT hr1 = conCheck.OpenContainer(file);
										if (hr1 != S_OK) {
											conCheck.CloseContainer();
											continue;
										}
										DWORD contentCheck = 0x0;
										if ((hr1 = conCheck.GetContentType(&contentCheck)) == S_OK){
											if (contentCheck == 0xB0000) {
												if (strcmp(wstrtostr(conFile.GetDisplayName()).c_str(), wstrtostr(conCheck.GetDisplayName()).c_str()) == 0)
												{
													FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), FindFileData.cFileName, "", 1, TUID, TUPaths.at(i)->pathId, 2 );
												}
											}
										}
										conCheck.CloseContainer();
									}

								}
							}
						}
						conFile.CloseContainer();
					}
				}
				FindClose(hFind);
			}
			szStatus = "";
			if(Function != 2 && Function != 3) {
				working = false;
				pause = true;
				Function = 0;
			}
		} else if (Function == 2)
		{
			DebugMsg("TitleUpdateManager", "Preparing Proper Updates to Launch [%08X]", TID);
			vector<SQLTitleUpdate*> TitleUpdate = FSDSql::getInstance().getTitleUpdatesForTitleId(0, TID);
			for (unsigned int x =0; x < TitleUpdate.size(); x++)
			{
				if (TitleUpdate.at(x)->isDisabled)
				{
					if (TitleUpdate.at(x)->isBackup)
					{
						string filepath = sprintfaA("%s\\%s", TitleUpdate.at(x)->path.c_str(), TitleUpdate.at(x)->filename.c_str());
						DebugMsg("TitleUpdateManager", "Deleteing File %s", filepath.c_str());
						_unlink(filepath.c_str());
						FSDSql::getInstance().deleteTitleUpdate(TitleUpdate.at(x)->titleUpdateId);
					}
						
				}
			}
			for (unsigned int x =0; x < TitleUpdate.size(); x++)
			{
				if (!TitleUpdate.at(x)->isDisabled)
				{
					if (!TitleUpdate.at(x)->isBackup)
					{
						bool contentFound = false;
						bool cacheFound = false;
						for(unsigned int y =0; y < TitleUpdate.size(); y++)
						{
							if (TitleUpdate.at(y)->isBackupFor == TitleUpdate.at(x)->titleUpdateId)
							{
								if (TitleUpdate.at(y)->pathType == 2)
									contentFound = true;
								else if (TitleUpdate.at(y)->pathType == 1)
									cacheFound = true;	
							}
						}
						if (!contentFound || !cacheFound)
						{
							TitleUpdatePath* path = FSDSql::getInstance().getTitleUpdatePathByUpdateID(TitleUpdate.at(x)->titleUpdateId);
							string sourcepath = sprintfaA("%s\\%s", TitleUpdate.at(x)->path.c_str(), TitleUpdate.at(x)->filename.c_str());
							if (!cacheFound) {
								string destpath = sprintfaA("%s\\%s", path->cachePath.c_str(), TitleUpdate.at(x)->filename.c_str());
								RecursiveMkdir(path->cachePath.c_str());
								if (destpath.find("tu") != string::npos)
								{
									if (FileExists(destpath))
										_unlink(destpath.c_str());
								}
								if (!FileExists(destpath))
								{
									CopyFile(sourcepath.c_str(), destpath.c_str(), false);
								}
							} 
							if (!contentFound)
							{
								string destpath = sprintfaA("%s\\%08X\\000B0000", path->contentPath.c_str(), TID);
								RecursiveMkdir(destpath);
								destpath = destpath + "\\" + TitleUpdate.at(x)->filename.c_str();
								if (destpath.find("tu") != string::npos)
								{
									if (FileExists(destpath))
										_unlink(destpath.c_str());
								}
								if (!FileExists(destpath))
								{
									CopyFile(sourcepath.c_str(), destpath.c_str(), false);
								}
							}
							if( path != NULL ) delete path;
						}
					}
				}
			}

			// Clean up our Titleupdate vectors
			for( unsigned int nCount = 0; nCount < TitleUpdate.size(); nCount++) {
				SQLTitleUpdate * update = TitleUpdate.at(nCount);
				if(update != NULL)
					delete update;
			}

			
			if (Function != 1 && Function != 3) {
				working = false;
				pause = true;
				Function = 0;
			}
		} else if (Function ==3 ) {
			DebugMsg("TitleUpdateManager", "Deleting TitleUpdate [%d] for TitleID [%08X]", TU->titleUpdateId, TID);
			vector<SQLTitleUpdate*> tuVec = FSDSql::getInstance().getTitleUpdatesForTitleId(0, TID);
			for (unsigned int x = 0; x < tuVec.size(); x++)
			{
				if (tuVec.at(x)->isBackupFor == TU->titleUpdateId ||
					tuVec.at(x)->titleUpdateId == TU->isBackupFor ) {
						string delFile = sprintfaA("%s\\%s", tuVec.at(x)->path.c_str(), tuVec.at(x)->filename.c_str());
						DebugMsg("TitleUpdateManager", "Deleteing [%s]", delFile.c_str());
						_unlink(delFile.c_str());
				}
			}
			string delFile = sprintfaA("%s\\%s", TU->path.c_str(), TU->filename.c_str());
			DebugMsg("TitleUpdateManager", "Deleteing [%s]", delFile.c_str());
			_unlink(delFile.c_str());
			working = false;
			if (Function != 1 && Function != 2) {
				working = false;
				pause = true;
				Function = 0;
			}
		}
	}
	return 0;
}

void TitleUpdateManager::FileScan(string path, string destdrive) {

	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd;
	searchcmd = sprintfaA("%s\\*", path.c_str());
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");

	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			szStatus = findFileData.cFileName;
			string file = findFileData.cFileName;
			string sourcepath;
			string destpath;
			if (file.find("tu") != string::npos)
			{
				DebugMsg("TitleUpdateManager", "Found title update [%s] backing it up.", findFileData.cFileName);
				string conFullPath = path + "\\" + file;
				Xbox360Container conFile;
				HRESULT hr = conFile.OpenContainer(conFullPath);
				if(hr != S_OK) {
					conFile.CloseContainer();
					continue;
				}

				// Determine what type of content this is
				DWORD contentType = 0x0;
				if((hr = conFile.GetContentType(&contentType)) == S_OK){
					if(contentType == 0xB0000) {
						destpath = sprintfaA("%s\\%s\\%s", destdrive.c_str(), wstrtostr(conFile.GetDisplayName()).c_str(), file.c_str());
					}
				}
				conFile.CloseContainer();
				if (!FileExists(destpath))
				{
					sourcepath = sprintfaA("%s\\%s", path.c_str(), file.c_str());
					CopyFile(sourcepath.c_str(), destpath.c_str(), false);
				}	
			} else if (file.find("TU") != string::npos) {
				destdrive = destdrive + "\\" + file;
				if (!FileExists(destdrive))
				{
					sourcepath = path + "\\" + file;
					CopyFile(sourcepath.c_str(), destdrive.c_str(), false);
				}
			}
		} while (FindNextFile(hFind, &findFileData));
	}
	FindClose(hFind);
}

void TitleUpdateManager::newTuFolderScan(TitleUpdatePath* path, string addPath)
{
	string basepath = path->dataPath + "\\" + addPath + "\\";
	DebugMsg("TitleUpdateManager", "Scanning for TitleUpdates [%s] [%d]", basepath.c_str(), path->pathId);
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd;
	searchcmd = sprintfaA("%s*", basepath.c_str());
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");

	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			szStatus = findFileData.cFileName;
			string file = findFileData.cFileName;
			if (file.find("TU") != string::npos || file.find("tu") != string::npos)
			{
				DebugMsg("TitleUpdateManager", "Found [%s] in [%s]", findFileData.cFileName, basepath.c_str());
				string conFullPath = basepath + findFileData.cFileName;
				Xbox360Container conFile;
				HRESULT hr = conFile.OpenContainer(conFullPath);
				if(hr != S_OK) {
					conFile.CloseContainer();
					continue;
				}

				// Determine what type of content this is
				DWORD contentType = 0x0;
				if((hr = conFile.GetContentType(&contentType)) == S_OK){
					if(contentType == 0xB0000) {
						DWORD TmpId;
						hr = conFile.GetTitleId(&TmpId);
						if (hr == S_OK)
						{
							DebugMsg("TitleUpdateManager", "Found TitleUpdate for TitleID [%s] [%08X]", wstrtostr(conFile.GetDisplayName()).c_str(), TmpId);
							ULONGLONG TUID = FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), findFileData.cFileName, addPath, 0, -1, path->pathId, 0 );
							string file = path->cachePath + "\\" + findFileData.cFileName;
							if (FileExists(file))
							{
								Xbox360Container conCheck;
								HRESULT hr1 = conCheck.OpenContainer(file);
								if (hr != S_OK) {
									conCheck.CloseContainer();
									continue;
								}
								DWORD contentCheck = 0x0;
								if ((hr1 = conCheck.GetContentType(&contentCheck)) == S_OK){
									if (contentCheck == 0xB0000) {
										if (strcmp(wstrtostr(conFile.GetDisplayName()).c_str(), wstrtostr(conCheck.GetDisplayName()).c_str()) == 0)
										{
											FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), findFileData.cFileName, "", 1, TUID, path->pathId, 1 );
										}
									}
								}
								conCheck.CloseContainer();
							}
							file = sprintfaA("%s\\%08X\\000B0000\\%s", path->contentPath.c_str(), TmpId, findFileData.cFileName);
							if (FileExists(file))
							{
								Xbox360Container conCheck;
								HRESULT hr1 = conCheck.OpenContainer(file);
								if (hr != S_OK) {
									conCheck.CloseContainer();
									continue;
								}
								DWORD contentCheck = 0x0;
								if ((hr1 = conCheck.GetContentType(&contentCheck)) == S_OK){
									if (contentCheck == 0xB0000) {
										if (strcmp(wstrtostr(conFile.GetDisplayName()).c_str(), wstrtostr(conCheck.GetDisplayName()).c_str()) == 0)
										{
											FSDSql::getInstance().addTitleUpdate(TmpId, conFile.GetDisplayName(), findFileData.cFileName, "", 1, TUID, path->pathId, 2 );
										}
									}
								}
								conCheck.CloseContainer();
							}

						}
					}
				}
				conFile.CloseContainer();
			}
		} while (FindNextFile(hFind, &findFileData));
	}
	FindClose(hFind);
}

void TitleUpdateManager::ScanCache(TitleUpdatePath* tPath)
{
	string path = tPath->cachePath;

	DebugMsg("TitleUpdateManager", "Looking for TitleUpdates in [%s] to backup", path.c_str());
	
	// Looking in Cache for any files to backup
	WIN32_FIND_DATA FindFileData;
	memset(&FindFileData, 0, sizeof(WIN32_FIND_DATA));

	if (path.c_str()[path.length() -1] != '\\')
	{
		path = path + "\\";
	}
	string SearchCmd = path + "*";

	HANDLE hFind = NULL;
	string FullFolder = "";
	bool FileFound = true;
	for (hFind = FindFirstFile(SearchCmd.c_str(), &FindFileData); (FileFound && (hFind != INVALID_HANDLE_VALUE)); FileFound = FindNextFile(hFind, &FindFileData))
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//do Nothing
		}
		else {
			szStatus =FindFileData.cFileName;
			string file = FindFileData.cFileName;
			string sourcepath;
			string destpath;
			if (file.find("tu") != string::npos)
			{
				string conFullPath = path + file;
				Xbox360Container conFile;
				HRESULT hr = conFile.OpenContainer(conFullPath);
				if(hr != S_OK) {
					conFile.CloseContainer();
					continue;
				}

				// Determine what type of content this is
				DWORD contentType = 0x0;
				if((hr = conFile.GetContentType(&contentType)) == S_OK){
					if(contentType == 0xB0000) {
						destpath = sprintfaA("%s\\%s\\%s", tPath->dataPath.c_str(), wstrtostr(conFile.GetDisplayName()).c_str(), file.c_str());
					}
				}
				conFile.CloseContainer();
				if (!FileExists(destpath))
				{
					sourcepath = conFullPath;
					CopyFile(sourcepath.c_str(), destpath.c_str(), false);
				}
			} else if (file.find("TU") != string::npos) {
				destpath = path + "\\" + file;
				if (!FileExists(destpath))
				{
					sourcepath = path + "\\" + file;
					CopyFile(sourcepath.c_str(), destpath.c_str(), false);
				}
			}
				
		}
	}
	FindClose(hFind);
}

void TitleUpdateManager::ScanContent(TitleUpdatePath* tPath)
{					
	string path = tPath->contentPath;
	DebugMsg("TitleUpdateManager", "Searching Content Folder");
	
	// Looking in Cache for any files to backup
	WIN32_FIND_DATA FindFileData;
	memset(&FindFileData, 0, sizeof(WIN32_FIND_DATA));

	if (path.c_str()[path.length() -1] != '\\')
	{
		path = path + "\\";
	}
	string SearchCmd = path + "*";

	HANDLE hFind = NULL;
	string FullFolder = "";
	bool FileFound = true;
	for (hFind = FindFirstFile(SearchCmd.c_str(), &FindFileData); (FileFound && (hFind != INVALID_HANDLE_VALUE)); FileFound = FindNextFile(hFind, &FindFileData))
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			szStatus = FindFileData.cFileName;
			string copypath = path + FindFileData.cFileName;
			copypath = copypath + "\\000B0000";
			if (FileExists(copypath))
				FileScan(copypath, tPath->dataPath);
		}
	}
	FindClose(hFind);
}