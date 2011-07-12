#pragma once
#include <sys/stat.h>
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../Generic/CustomMessages.h"


class FileItem {
public:
	FileItem()
	{
		size.QuadPart = 0;
		IsDir = false;
		IsSelected = false;
	}

	void CreateWStrs()
	{
		wname = strtowstr(name);
		if (IsDir)
		{
			wsize = L"";
		} else {
			if(size.QuadPart <= 1024)
            {
                wsize = strtowstr(sprintfaA("%i byte",size.QuadPart));
            }
            else if (size.QuadPart > 1024 && size.QuadPart <= (1024 * 1024))
            {
                wsize = strtowstr(sprintfaA("%0.1f kB",(float)size.QuadPart/1024.0f));
            }
            else if (size.QuadPart > (1024 * 1024) && size.QuadPart <= (1024 * 1024 * 1024))
            {
                wsize = strtowstr(sprintfaA("%0.1f MB",(float)size.QuadPart/(1024.0f*1024.0f)));
            }
            else
            {
                wsize = strtowstr(sprintfaA("%0.1f GB",(float)size.QuadPart/(1024.0f*1024.0f*1024.0f)));
            }
		}
	}

	bool IsDir;
	bool IsSelected;
	LARGE_INTEGER size;
	string name;
	wstring wname;
	wstring wsize;
};

class FileBrowser
{
protected :
	vector<string> m_CurrentFolder;

	string mGetFileFTPLongDescription(string file);
	string mGetFolderFTPLongDescription(string folder);

private :
	string FlipToBackwardSlashes(string pPath);
	HXUIOBJ parentScene;  // used only to request smb: credentials when we don't have them

public :
	 FileBrowser()
	 {
		 parentScene = NULL;
	 };
	 
	 void setParentScene(HXUIOBJ h) { parentScene = h; }
	 string GetCurrentPath();
	 string GetRawSmbPath(string& user, string& password, string& share);

	 int GetCurrentPathDepth();
	 bool IsAtRoot();
	 virtual string GetWriteFilePath(string file);
	 virtual FILE* OpenFile(string file);
	 virtual string GetFileFTPLongDescription(string file);
	 virtual string GetFolderFTPLongDescription(string folder);
	 virtual vector<string> GetFolderList();
	 virtual vector<string> GetFileList();
	 virtual void DebugOut();
	 void UpToRoot();
	 void UpDirectory();
	 virtual BOOL DeleteFolder(string folder);
	 void CD(string folder);
};