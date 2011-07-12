#include "stdafx.h"

#include "FileBrowserBase.h"
#include "../Debug/Debug.h"

bool FileBrowserBase::UpToRoot()
{
	if (!IsAtRoot())
	{
		currentDirectory.clear();
		return true;
	}
	return false;
}

bool FileBrowserBase::UpDirectory()
{
	if(!IsAtRoot())
	{
		currentDirectory.pop_back();
		return true;
	}
	return false;
}

string FileBrowserBase::GetCurrentPath()
{
	string currentPath(currentRoot);
	
	for each (string dir in currentDirectory)
	{
		currentPath += "\\" + dir;
	}

	DebugMsg("FileBrowserBase", "GetCurrentPath(): %s", currentPath.c_str());
	
	return currentPath;
}

bool FileBrowserBase::IsAtRoot()
{
	return (currentDirectory.size() == 0);
}

bool FileBrowserBase::ChangeDirectory(string newDirectory)
{
	DebugMsg("FileBrowserBase", "newDirectory at function entry: '%s'", newDirectory.c_str());

	if (newDirectory.length() == 0) return false;
	
	string fullPath = "";
	vector<string> splitPath;
	
	//check if dir starts with backslash (root)
	if (newDirectory.substr(0, 1) == "\\")
	{
		newDirectory = currentRoot + newDirectory;
	}
	
	// strip trailing backslash
	if (newDirectory.substr(newDirectory.length() - 1, 1) == "\\")
	{
		newDirectory = newDirectory.substr(0, newDirectory.length() - 1);
	}
	
	//check if the path does not contain a drive
	if (newDirectory.substr(0, newDirectory.find("\\")).find(":") == string::npos)
	{
		newDirectory = GetCurrentPath() + "\\" + newDirectory;
	}
	
	//re-add the backslash if its a drive root
	if (newDirectory.find("\\") == newDirectory.npos)
	{
		newDirectory.append("\\");
	}
	
	//check if exists AND is a directory
	if (IsExistingDirectory(newDirectory))
	{
		DebugMsg("FileBrowserBase", "newDirectory '%s' is a valid directory, changing to it", newDirectory.c_str());
		StringSplit(newDirectory, "\\", &splitPath);
		if (!splitPath.empty())
		{
			currentRoot = splitPath.at(0);
		}
		if (splitPath.size() > 1)
		{
			currentDirectory.assign(splitPath.begin() + 1, splitPath.end());
		}
		else
		{
			currentDirectory.clear();
		}
		return true;
	}
	else
	{
		DebugMsg("FileBrowserBase", "newDirectory '%s' is not a (valid) directory", newDirectory.c_str());
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

SystemFileBrowser::SystemFileBrowser()
{
	//set root path to Game:
	currentRoot = "Game:";
}

bool SystemFileBrowser::IsExistingDirectory(string directoryName)
{
	DWORD attrib = GetFileAttributes(directoryName.c_str());
	return ((attrib != 0xFFFFFFFF) && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool SystemFileBrowser::IsExistingFile(string fileName)
{
	DWORD attrib = GetFileAttributes(fileName.c_str());
	return ((attrib != 0xFFFFFFFF) && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool SystemFileBrowser::GetDirectoryAndFileLists(vector<string>* directoryList, vector<string>* fileList)
{
	if (directoryList == NULL && fileList == NULL) return false;

	if (directoryList != NULL) directoryList->clear();
	if (fileList != NULL) fileList->clear();

	WIN32_FIND_DATA findFileData;
	memset(&findFileData, 0, sizeof(WIN32_FIND_DATA));
	string searchCmd = GetCurrentPath() + "\\*";

	DebugMsg("SystemFileBrowser", "GetDirectoryAndFileLists for '%s'", searchCmd.c_str());
	
	HANDLE hFind = FindFirstFile(searchCmd.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE) return false;
	do
	{
		if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (directoryList != NULL)
			{
				directoryList->push_back(findFileData.cFileName);
				DebugMsg("SytemFileBrowser", "Added to directoryList: '%s'", findFileData.cFileName);
			}
		}
		else
		{
			if (fileList != NULL)
			{
				fileList->push_back(findFileData.cFileName);
				DebugMsg("SytemFileBrowser", "Added to fileList: '%s'", findFileData.cFileName);
			}
		}
	} while (FindNextFile(hFind, &findFileData));
	
	FindClose(hFind);
	
	return true;
}
string SystemFileBrowser::GetCurrentRealPath()
{
	return GetCurrentPath();
}
////////////////////////////////////////////////////////////////////////////////

ContentFileBrowser::ContentFileBrowser()
{
	//set root path to Xbox360:
	currentRoot = "Xbox360:";
}

bool ContentFileBrowser::IsExistingDirectory(string directoryName)
{
	return true;
}

bool ContentFileBrowser::IsExistingFile(string fileName)
{
	return true;
}

bool ContentFileBrowser::GetDirectoryAndFileLists(vector<string>* directoryList, vector<string>* fileList)
{
	return false;
}
string ContentFileBrowser::GetCurrentRealPath()
{
	return "";
}

////////////////////////////////////////////////////////////////////////////////

FileBrowserFactory::FileBrowserFactory(FileBrowserType fbType)
{
	currentType = fbType;
	switch (fbType)
	{
		case System:
			fileBrowser = new SystemFileBrowser();
		case Content:
			fileBrowser = new ContentFileBrowser();
	}
}

FileBrowserFactory::~FileBrowserFactory()
{
	delete fileBrowser;
}

FileBrowserBase* FileBrowserFactory::getFileBrowser()
{
	return fileBrowser;
}

bool FileBrowserFactory::SwitchType(FileBrowserType fbType)
{
	if (fbType != currentType)
	{
		currentType = fbType;
		switch(fbType)
		{
			case System:
				fileBrowser = new SystemFileBrowser();
				//try to convert current path?
				break;
			case Content:
				fileBrowser = new ContentFileBrowser();
				//try to convert current path?
				break;
		}
		return true;
	}	
	return false;
}

FileBrowserFactory::FileBrowserType FileBrowserFactory::GetType()
{
	return currentType;
}