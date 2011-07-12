#include "stdafx.h"

#include "../../Application/FreestyleApp.h"
#include "FileBrowser.h"
#include "../Managers/SambaClient/SambaClient.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../Managers/Drives/Drive.h"

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../../../Libs/libsmbd/xbox/xbLibSmb.h"
#include "../Settings/Settings.h"

string FileBrowser::FlipToBackwardSlashes(string pPath)
{
	string Result(pPath);
	size_t Pos = 0;
	
	while ( (Pos = Result.find("/", Pos)) != Result.npos )
	{
		Result.replace( Pos, 1, "\\" );
		Pos++;
	}
	return Result;
}

void FileBrowser::UpToRoot()
{
	m_CurrentFolder.clear();
}

void FileBrowser::UpDirectory()
{
	if(m_CurrentFolder.size() > 0)
	{
		m_CurrentFolder.pop_back();
	}
}

bool fexists(string f)
{
	return (GetFileAttributes(f.c_str()) != 0xFFFFFFFF);
}

void FileBrowser::CD(string folder)
{
	//Mattie: quickfix going up to root... ftp '/' is converted to ':'
	if(strcmp(folder.c_str(), ":") == 0)
	{
		UpToRoot();
		return;
	}
	
	if(strcmp(folder.c_str(), "..") == 0)
	{
		UpDirectory();
	}
	else
	{
	/*	if (folder.c_str()[0] != '\\')
		{
			folder = GetCurrentPath()
	*/	
		if(folder.c_str()[0] == '/')
		{
			UpToRoot();
			vector<string> splits;
			StringSplit(folder,"/",&splits);
			for(unsigned int x=0;x<splits.size();x++)
			{
				string f = splits.at(x);
				if(m_CurrentFolder.size() == 0)
				{
					if(f.c_str()[f.size()-1] != ':')
					{
						f = f + ":";
					}
				
				}
				m_CurrentFolder.push_back(f);
			}
		}
		else
		{
			if (folder.substr(0,4).compare("smb:") == 0 || GetCurrentPath().substr(0,4).compare("smb:") == 0) {
//			if (folder.compare("smb:") == 0) {
//				DebugMsg("FileBrowser","Entering smb:");
//				m_CurrentFolder.push_back(folder);
//				return;
			//}
				
				vector<string> splits;
				StringSplit(folder,"/",&splits);
				if(splits.size()>1)
				{
					UpToRoot();
					for(unsigned int x=0;x<splits.size();x++) //Dont do the file
					{
						string f = splits.at(x);
						CD(f);
					}
				}
				else
				{
					DebugMsg("FileBrowser","Entering smb:");
					m_CurrentFolder.push_back(folder);
				}
				return;
			}
			//mattie: quickfix: dont allow folders that dont exist, cut trailing backslash
			//// if it ends in :, add \  ie.  "hdd1: -> hdd1:\"
			if (folder.substr(folder.length() - 1, 1) == ":") folder.append("\\");
			//// if the folder doesn't exist and there's a : in it, return  ie. hdd1:\foobar , but not foobar
			if (!fexists(folder) && (folder.find(":") != folder.npos)) return; // if (!FileExistsA(folder)) return;
			//// if the folder ends in \, remove the slash  ie. hdd1:\foo\ -> hdd1:\foo, but also hdd1:\ -> hdd1:
			
			if (folder.compare("\\") == 0) {
				m_CurrentFolder.clear();
				return;
			}
			
			if (folder.substr(folder.length() - 1, 1) == "\\") folder = folder.substr(0, folder.length() - 1);
			// if the folder ends in :, call UpToRoot()
			if (folder.substr(folder.length() - 1, 1) == ":") UpToRoot();

			vector<string> splits;
			StringSplit(folder,"\\",&splits);
			if(splits.size()>1)
			{
				UpToRoot();
				for(unsigned int x=0;x<splits.size();x++) //Dont do the file
				{
					string f = splits.at(x);
					CD(f);
				}
			}
			else
			{
				if(m_CurrentFolder.size() == 0)
				{
					if(folder.c_str()[folder.size()-1] != ':')
					{
						folder = folder + ":";
					}
				}	
				m_CurrentFolder.push_back(folder);
			}
		}
	}
 }

bool FileBrowser::IsAtRoot()
{
	if(m_CurrentFolder.size() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BOOL FileBrowser::DeleteFolder(string folder)
{
	//Create a new filebrowser with same path...
	FileBrowser temp;
	temp.CD(GetCurrentPath());
	temp.CD(folder);
	
	vector<string> files = temp.GetFileList();
	for(unsigned int x=0;x<files.size();x++)
	{
		string fullFile = temp.GetCurrentPath() + "\\" + files.at(x);
		unlink(fullFile.c_str());
	}
	vector<string> folders = temp.GetFolderList();
	for(unsigned int x=0;x<folders.size();x++)
	{
		//prevent whole mountpoint getting wiped!
		if (folders.at(x) != "..")
		{
			temp.DeleteFolder(folders.at(x));
		}
	}
	return RemoveDirectory (temp.GetCurrentPath().c_str());
}

int FileBrowser::GetCurrentPathDepth() 
{
	return m_CurrentFolder.size();
}

string FileBrowser::GetCurrentPath()
{
	string retVal = "";
	bool smbFlag = false;
	bool bSupress = false;
	string smbCred = "";

	for(unsigned int x=0;x<m_CurrentFolder.size();x++)
	{
		if (x == 0 && m_CurrentFolder.at(x).compare("smb:") == 0) {
			smbFlag = true;
		}

		if(x!=0)
		{
			if (smbFlag) {
				if (x != 1 && !bSupress) {
					retVal.append("/");
				}
				bSupress = false;
			}
			else
				retVal.append("\\");
		}

		if (smbFlag) {
			if (x > 0) {
				if (x == 1 && m_CurrentFolder.size() > 2) {
					map<string, unsigned int>::iterator it = SambaClient::getInstance().m_SambaTopLevelTypes.find(m_CurrentFolder.at(1));
					if (it != SambaClient::getInstance().m_SambaTopLevelTypes.end()) {
						if (it->second == SMBC_WORKGROUP) {
							bSupress = true;
							continue;
						}
					}
				}
				retVal = retVal + m_CurrentFolder.at(x);	

				if (x == 2) {
					if (m_CurrentFolder.at(1).find("@") == m_CurrentFolder.at(x).npos) {
						string lookup = m_CurrentFolder.at(1) + "/" + m_CurrentFolder.at(2);
						SambaCredentials cred;
						if (SambaClient::getInstance().GetSambaCredentials(lookup, cred))
						{
							smbCred = "/" + cred.user + ":" + cred.password + "@";
						}
						else {
							smbCred = "/guest:@";
						}
					}
				}
			}
		}
		else
			retVal = retVal + m_CurrentFolder.at(x);		

	}
	if (smbFlag) {
		if (smbCred == "" && retVal != "")
			smbCred = "/";
		retVal = "smb:/" + smbCred + retVal;
	}
	return retVal;
}

string FileBrowser::GetRawSmbPath(string& user, string& password, string& share)
{
	string retVal = "";
	bool smbFlag = false;
	bool bSupress = false;
	string smbCred = "";

	for(unsigned int x=0;x<m_CurrentFolder.size();x++)
	{
		if (x == 0 && m_CurrentFolder.at(x).compare("smb:") == 0) {
			smbFlag = true;
		}

		if(x!=0)
		{
			if (smbFlag) {
				if (x != 1 && !bSupress) {
					retVal.append("/");
				}
				bSupress = false;
			}
			else
				retVal.append("\\");
		}

		if (smbFlag) {
			if (x > 0) {
				if (x == 1 && m_CurrentFolder.size() > 2) {
					map<string, unsigned int>::iterator it = SambaClient::getInstance().m_SambaTopLevelTypes.find(m_CurrentFolder.at(1));
					if (it != SambaClient::getInstance().m_SambaTopLevelTypes.end()) {
						if (it->second == SMBC_WORKGROUP) {
							bSupress = true;
							continue;
						}
					}
				}
				retVal = retVal + m_CurrentFolder.at(x);	

				if (x == 2) {
					if (m_CurrentFolder.at(1).find("@") == m_CurrentFolder.at(x).npos) {
						string lookup = m_CurrentFolder.at(1) + "/" + m_CurrentFolder.at(2);
						SambaCredentials cred;
						share = lookup;
						if (SambaClient::getInstance().GetSambaCredentials(lookup, cred))
						{
							user = cred.user;
							password = cred.password;
						}
					}
				}
			}
		}
		else
			retVal = retVal + m_CurrentFolder.at(x);		

	}
	if (smbFlag) {
		if (smbCred == "" && retVal != "")
			smbCred = "/";
		retVal = "smb:/" + smbCred + retVal;
	}
	return retVal;
}


void FileBrowser::DebugOut()
{
	DebugMsg("FileBrowser","Listing %s",GetCurrentPath().c_str());
	vector<string> folders = GetFolderList();
	DebugMsg("FileBrowser","Folders : ");
	for(unsigned int x=0;x<folders.size();x++)
	{
		DebugMsg("FileBrowser","%s",folders.at(x).c_str());
	}
	vector<string> files = GetFileList();
	DebugMsg("FileBrowser","Files : ");
	for(unsigned int x=0;x<files.size();x++)
	{
		DebugMsg("FileBrowser","%s",files.at(x).c_str());
	}
}
string FileBrowser::GetWriteFilePath(string file)
{
	string fullPath = GetCurrentPath()+"\\" + file;
	return fullPath;
}
FILE* FileBrowser::OpenFile(string file)
{
	FILE* retVal;
	string fullPath = GetCurrentPath()+"\\" + file;
	fopen_s(&retVal,fullPath.c_str(),"rb");
	return retVal;
}

static bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() > ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

vector<string> FileBrowser::GetFolderList()
{
	vector<string> retVal;
	if(m_CurrentFolder.size()==0)
	{
		//List Drives
		retVal.push_back("Game:");

		if (SETTINGS::getInstance().getSambaClientOn()) 
			retVal.push_back("smb:");

		std::vector<Drive* const> mountedDrives;
	
		DrivesManager::getInstance().getMountedDrives(&mountedDrives);
		for(unsigned int x=0;x<mountedDrives.size();x++)
		{

			retVal.push_back(mountedDrives[x]->GetCleanDriveName() + ":");
		}

		return retVal;
	}
	else
	{
		retVal.push_back("..");

		string path = GetCurrentPath();
		if (path.substr(0,4).compare("smb:") == 0) {
			map<string, unsigned int> m_SambaTypes;

			int dir;
			if (path.length() == 5) {
				if (SambaClient::getInstance().m_SambaTopLevelTypes.size() > 0) {
					for (map<string, unsigned int>::const_iterator it = SambaClient::getInstance().m_SambaTopLevelTypes.begin(); it != SambaClient::getInstance().m_SambaTopLevelTypes.end(); ++it)
					{
						retVal.push_back(it->first);
					}
					return retVal;
				}
			}
			path.append("/");
			if ((dir = smbc_opendir(path.c_str())) < 0)
			{
				DebugMsg("FileBrowser", "Could not open directory [%s] (%d:%s)\n", path.c_str(), errno, strerror(errno));
				if (errno == 13) {  // permission denied
					string user;
					string password;
					string share;
					string rawpath = GetRawSmbPath(user, password, share);

					XUIMessage xuiMsg;
					XuiMessage(&xuiMsg, XM_SMB_PERMISSION_DENIED);
					Credentials * cred = new Credentials();
					_XuiMessageExtra(&xuiMsg,(XUIMessageData*) cred, sizeof(*cred));
					cred->smbPath = rawpath;
					cred->user = user;
					cred->password = password;
					cred->share = share;

					XuiSendMessage( parentScene , &xuiMsg );
				}
				return retVal;
			}
			struct smbc_dirent *        dirent;
		    while ((dirent = smbc_readdir(dir)) != NULL)
			{
				if (dirent->smbc_type != SMBC_WORKGROUP &&
					dirent->smbc_type != SMBC_SERVER &&
					dirent->smbc_type != SMBC_FILE_SHARE &&
					dirent->smbc_type != SMBC_DIR)
					continue;
				if (strcmp(dirent->name, ".") == 0 || strcmp(dirent->name, "..") == 0)
					continue;
				if (hasEnding(dirent->name, "$"))
					continue;

				if (m_CurrentFolder.size() == 1) {
					SambaClient::getInstance().m_SambaTopLevelTypes[dirent->name] = dirent->smbc_type;
				}

				// filter ourselves out of the list of servers
				if (SETTINGS::getInstance().getSambaServerOn() && CFreestyleApp::getInstance().hasInternetConnection()) {
					if (m_CurrentFolder.size() == 2) {
						string wrkgroup = SETTINGS::getInstance().getSambaClientWorkgroup();
						if (stricmp(wrkgroup.c_str(), m_CurrentFolder[1].c_str()) == 0) {
							string hostname = SETTINGS::getInstance().getHostname();
							if (stricmp(hostname.c_str(), dirent->name) == 0)
								continue;
						}
					}
				}
				retVal.push_back(dirent->name);
			}
//			if (m_CurrentFolder.size() == 1) {
//				m_SambaTopLevelTypes = m_SambaTypes;
//				//m_CurrentFolderSambaTypes.push_back(m_SambaTypes);
//			}
			smbc_closedir(dir);
			return retVal;
		}
		WIN32_FIND_DATA findFileData;
		memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
		string searchcmd = GetCurrentPath() + "\\*";
	
		HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return retVal;
		do {
			string s = findFileData.cFileName;
			if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				retVal.push_back(s);
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
		return retVal;
	}

}
string FileBrowser::GetFileFTPLongDescription(string file)
{
	return mGetFileFTPLongDescription(file);
}
string FileBrowser::mGetFileFTPLongDescription(string file)
{
	string out = "";

	struct stat statFileStatus;
	string fullFilePath = GetCurrentPath() + "\\" + file;
	stat(fullFilePath.c_str(),&statFileStatus);
		
		
	DWORD attr = GetFileAttributes(fullFilePath.c_str());
	char execute = 'x';
	char read = 'r';
	char write = 'w';
	char directory = '-';
	if(attr&FILE_ATTRIBUTE_DIRECTORY)
	{
		directory = '-';
		directory = 'd';
	}
	if(attr&FILE_ATTRIBUTE_ARCHIVE )
	{
		execute = '-';
	}
	if(attr&FILE_ATTRIBUTE_READONLY  )
	{
		write = '-';
	}
	if(attr&FILE_ATTRIBUTE_DEVICE   )
	{
		read = '-';
	}
	char timeStr[ 100 ] = "";
	struct tm locTime;
	int retval = localtime_s(&locTime, &statFileStatus.st_mtime);
	if(retval)
	{
		retval = localtime_s(&locTime, &statFileStatus.st_atime);
	}
	if(retval)
	{
		retval = localtime_s(&locTime, &statFileStatus.st_ctime);
	}
	if(retval)
	{
		time_t t;
		time(&t);
		retval = localtime_s(&locTime, &t);
	}

/*	SYSTEMTIME stCutoff;
	GetSystemTime(&stCutoff);
	stCutoff.wYear--;

	if (locTime.tm_year+1900 > stCutoff.wYear || 
		(locTime.tm_year+1900 == stCutoff.wYear && locTime.tm_mon > stCutoff.wMonth) ||
		(locTime.tm_year+1900 == stCutoff.wYear && locTime.tm_mon == stCutoff.wMonth && locTime.tm_mday > stCutoff.wDay)) {
		strftime(timeStr, 100, "%b %d %Y",  &locTime);
	}
	else {*/
		strftime(timeStr, 100, "%b %d %H:%M",  &locTime);
//	}

	out= sprintfa("%c%c%c%c%c%c%c%c%c%c   1 root  root    %d %s %s\r\n",directory,read,write,execute,read,write,execute,read,write,execute,statFileStatus.st_size,timeStr,file.c_str());
	return out;
}
string FileBrowser::GetFolderFTPLongDescription(string folder)
{
	return mGetFolderFTPLongDescription(folder);
}
string FileBrowser::mGetFolderFTPLongDescription(string folder)
{
	string fullPath ="";
	if(m_CurrentFolder.size() ==0)
	{
		fullPath = folder + ":\\";
	}
	else
	{
		fullPath= GetCurrentPath() + "\\" + folder;
	}
	if(FileExists(fullPath))
	{
		DWORD attr = GetFileAttributes(fullPath.c_str());
		char execute = 'x';
		char read = 'r';
		char write = 'w';
		char directory = '-';
		if(attr&FILE_ATTRIBUTE_DIRECTORY)
		{
			directory = 'd';
		}
		if(attr&FILE_ATTRIBUTE_ARCHIVE )
		{
			execute = '-';
		}
		if(attr&FILE_ATTRIBUTE_READONLY  )
		{
			write = '-';
		}
		if(attr&FILE_ATTRIBUTE_DEVICE   )
		{
			read = '-';
		}
		struct stat statDirStatus;
		stat(fullPath.c_str(),&statDirStatus);
		char timeStr[ 100 ] = "";
		struct tm locTime;
		int retval = localtime_s(&locTime, &statDirStatus.st_mtime);
		if(retval)
		{
			retval = localtime_s(&locTime,  &statDirStatus.st_atime);
		}
		if(retval)
		{
			retval = localtime_s( &locTime, &statDirStatus.st_ctime);
		}
		if(retval)
		{
			time_t t;
			time(&t);
			retval = localtime_s(&locTime,  &t);
		}
		
		strftime(timeStr, 100, "%b %d %Y", &locTime);
		
		return sprintfa("%c%c%c%c%c%c%c%c%c%c   1 root  root    %d %s %s\r\n",directory,read,write,execute,read,write,execute,read,write,execute,0,timeStr,folder.c_str());
	}
	else
	{
		return sprintfa("%s\r\n",folder.c_str());
	}
}
vector<string> FileBrowser::GetFileList()
{
	DebugMsg("FileBrowser", "In Get FileList");
	vector<string> retVal;
	if(m_CurrentFolder.size()>0)
	{
		string path = GetCurrentPath();
		if (path.substr(0,4).compare("smb:") == 0) {
			map<string, unsigned int> m_SambaTypes;

			if (path.length() == 5) 
				return retVal;   // there are no "FILES" at the top level

			path.append("/");
			int dir;
			DebugMsg("FileBrowser", "calling opendir on %s", path.c_str());
			if ((dir = smbc_opendir(path.c_str())) < 0)
			{
				DebugMsg("FileBrowser", "Could not open directory [%s] (%d:%s)\n", path.c_str(), errno, strerror(errno));
				if (errno == 13) {
					retVal.push_back("*Permission Denied*");
				}
				return retVal;
			}
			struct smbc_dirent *dirent;
			DebugMsg("FileBrowser", "calling smbc_readdir on %s", path.c_str());
		    while ((dirent = smbc_readdir(dir)) != NULL)
			{
				if (dirent->smbc_type != SMBC_FILE)
					continue;
				if (strcmp(dirent->name, ".") == 0 || strcmp(dirent->name, "..") == 0)
					continue;
//				DebugMsg("FileBrowser","Inserting %s as type %d", dirent->name, dirent->smbc_type);
//				m_SambaTypes[dirent->name]  = dirent->smbc_type;

				retVal.push_back(dirent->name);
			}
			smbc_closedir(dir);
//			if (m_CurrentFolderSambaTypes.size() < m_CurrentFolder.size()) {
//				m_CurrentFolderSambaTypes.push_back(m_SambaTypes);
//			}

			return retVal;
		}



		WIN32_FIND_DATA findFileData;
		memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
		string searchcmd = GetCurrentPath() + "\\*";
	
		HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE)
			return retVal;
		do {
			string s = findFileData.cFileName;
			if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				
			}
			else
			{
				if(strcmp(s.c_str(),"..")!=0)
				{
				retVal.push_back(s);
				}
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
	return retVal;
}