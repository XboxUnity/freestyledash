#include "stdafx.h"

#include "VirtualFileBrowser.h"
#include "../../Managers/Drives/DrivesManager.h"
#include "../../Managers/Drives/Drive.h"
#include "../../ContentList/ContentItemNew.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"

void VirtualFileBrowser::ContentItemAdded(ContentItemNew * pContent)
{
	Content.push_back(pContent);
}

void VirtualFileBrowser::ContentItemMassAdd(vector<ContentItemNew*> items) 
{
	vector<ContentItemNew*>::iterator iter;
	for(iter = items.begin(); iter != items.end(); ++iter) {
		ContentItemNew * pContent = (*iter);
		Content.push_back(pContent);
	}
}


BOOL VirtualFileBrowser::DeleteFolder(string folder)
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
		//prevent whole mountpoint getting whiped!
		if (folders.at(x) != "..")
		{
			temp.DeleteFolder(folders.at(x));
		}
	}
	return false;
}
string VirtualFileBrowser::GetFileFTPLongDescription(string file)
{
	if(m_CurrentFolder.size() >= 2 && (m_CurrentFolder.at(0) == "360:" || m_CurrentFolder.at(0) == "XBLA:" || m_CurrentFolder.at(0) == "HOMEBREW:" || m_CurrentFolder.at(0) == "XBOX:" || m_CurrentFolder.at(0) == "EMULATORS:"))
	{
		string folder = GetRealCurrentPath();
		FileBrowser fb;
		fb.CD(folder);
		return fb.GetFileFTPLongDescription(file);
	}
	else
	{
		return sprintfa("-r--r--r--   1 root  root    %d 1 1 1970 %s\r\n",0,file.c_str());
	}
	
}
string VirtualFileBrowser::GetFolderFTPLongDescription(string folder)
{
	if(m_CurrentFolder.size() >= 2 && (m_CurrentFolder.at(0) == "360:" || m_CurrentFolder.at(0) == "XBLA:" || m_CurrentFolder.at(0) == "HOMEBREW:" || m_CurrentFolder.at(0) == "XBOX:" || m_CurrentFolder.at(0) == "EMULATORS:"))
	{
		string path = GetRealCurrentPath();
	
		FileBrowser fb;
		fb.CD(path);
		return fb.GetFolderFTPLongDescription(folder);
	}
	else
	{
		return sprintfa("dr--r--r--   1 root  root    %d 1 1 1970 %s\r\n",0,folder.c_str());
	}
}
void VirtualFileBrowser::DebugOut()
{

}
string VirtualFileBrowser::GetRealCurrentPath()
{
	if(m_CurrentFolder.size() >= 2 && (m_CurrentFolder.at(0) == "360:" || m_CurrentFolder.at(0) == "XBLA:" || m_CurrentFolder.at(0) == "HOMEBREW:" || m_CurrentFolder.at(0) == "XBOX:" || m_CurrentFolder.at(0) == "EMULATORS:"))
	{
		string currentFolder = m_CurrentFolder.at(0);
		string realPath = "";
		string gameName = m_CurrentFolder.at(1);
		CONTENT_ITEM_TAB listType = CONTENT_UNKNOWN;

		if(currentFolder == "360:")
		{
			listType = CONTENT_360;
		}
		if(currentFolder == "XBLA:")
		{
			listType = CONTENT_XBLA;
		}
		if(currentFolder == "HOMEBREW:")
		{
			listType = CONTENT_HOMEBREW;
		}
		if(currentFolder == "XBOX:")
		{
			listType = CONTENT_XBOX1;
		}
		if(currentFolder == "EMULATORS:")
		{
			listType = CONTENT_EMULATOR;
		}

		//Find Current Game
		vector <ContentItemNew*>::iterator itr;
		for (itr = Content.begin() ; itr != Content.end() ; itr++)
		{
			ContentItemNew * item = (*itr);

			if (item->GetItemTab() == listType && GameNameToFolderNameW(item->getTitle()) == strtowstr(gameName))
			{
				realPath = item->getRoot() + item->getPath();
				break;

			}
		}
		for(int x= 2;x < (int)m_CurrentFolder.size();x++)
		{
			realPath = realPath + m_CurrentFolder.at(x) +"\\";
		}
		return realPath;
	}
	else
	{
		return "";
	}
	return "";
}
string VirtualFileBrowser::GetWriteFilePath(string file)
{
	if(m_CurrentFolder.size() >= 2 && (m_CurrentFolder.at(0) == "360:" || m_CurrentFolder.at(0) == "XBLA:" || m_CurrentFolder.at(0) == "HOMEBREW:" || m_CurrentFolder.at(0) == "XBOX:" || m_CurrentFolder.at(0) == "EMULATORS:"))
	{
		string path = GetRealCurrentPath();
	
		FileBrowser fb;
		fb.CD(path);
		return fb.GetWriteFilePath(file);
	}
	else
	{
		return "";
	}
}
FILE* VirtualFileBrowser::OpenFile(string file)
{
	if(m_CurrentFolder.size() >= 2 && (m_CurrentFolder.at(0) == "360:" || m_CurrentFolder.at(0) == "XBLA:" || m_CurrentFolder.at(0) == "HOMEBREW:" || m_CurrentFolder.at(0) == "XBOX:" || m_CurrentFolder.at(0) == "EMULATORS:"))
	{
		string path = GetRealCurrentPath();
	
		FileBrowser fb;
		fb.CD(path);
		return fb.OpenFile(file);
	}
	else
	{
		return NULL;
	}
}
vector<string> VirtualFileBrowser::GetFolderList()
{
	vector<string> retVal;
	if(m_CurrentFolder.size()==0)
	{
		//List Drives
		retVal.push_back("Xbox360 Games");
		retVal.push_back("Xbox Live Arcade");
		retVal.push_back("Homebrew Games");
		retVal.push_back("Xbox Classic");
		retVal.push_back("Emulators");
		retVal.push_back("Settings");

		return retVal;
	}
	else
	{
		retVal.push_back("..");
		if(m_CurrentFolder.size()>0)
		{
			DebugMsg("VirtualFileBrowser","Currently at : %s",GetCurrentPath().c_str());
			string currentFolder = m_CurrentFolder.at(0);
			if(currentFolder == "Xbox360 Games" || currentFolder == "Xbox Live Arcade" || currentFolder == "Homebrew Games" || currentFolder == "Xbox Classic" || currentFolder == "Emulators")
			{
				int listType = -1;
				if(currentFolder == "Xbox360 Games")
				{
					DebugMsg("VirtualFileBrowser","Type is 360");
					listType = CONTENT_360;
				}
				if(currentFolder == "Xbox Live Arcade")
				{
					DebugMsg("VirtualFileBrowser","Type is xbla");
					listType = CONTENT_XBLA;
				}
				if(currentFolder == "Homebrew Games")
				{
					DebugMsg("VirtualFileBrowser","Type is homebrew");
					listType = CONTENT_HOMEBREW;
				}
				if(currentFolder == "Xbox Classic")
				{
					DebugMsg("VirtualFileBrowser","Type is xbox1");
					listType = CONTENT_XBOX1;
				}
				if(currentFolder == "Emulators")
				{
					DebugMsg("VirtualFileBrowser","Type is emulator");
					listType = CONTENT_EMULATOR;
				}
				
				if(m_CurrentFolder.size() == 1)
				{
					//List Games
					vector <ContentItemNew*>::iterator itr;
					for (itr = Content.begin() ; itr != Content.end() ; itr++)
					{
						ContentItemNew * item = (*itr);
						
						if (item->GetItemTab() == listType)
						{
							
							retVal.push_back(wstrtostr(GameNameToFolderNameW(item->getTitle())));
						}

						
					}
				}
				else if(m_CurrentFolder.size() >= 2)
				{
					string path  = GetRealCurrentPath();
					DebugMsg("VirtualFileBrowser","CurrentGamePath = %s",path.c_str());
					FileBrowser fbr;
					fbr.CD(path);
					vector<string> subFolders = fbr.GetFolderList();
					for(int x=0; x < (int)subFolders.size();x++)
					{
						retVal.push_back(subFolders.at(x));
					}
				}
			}
			
		}
		return retVal;
	}

}
string VirtualFileBrowser::GameNameToFolderName(string gameName)
{

	gameName = str_removeInvalidChar(gameName);
	return str_replaceA(gameName, ":", "");
}

wstring VirtualFileBrowser::GameNameToFolderNameW(wstring gameName)
{

	gameName = str_removeInvalidCharW(gameName);
	return str_replaceW(gameName, L":", L"");
}


vector<string> VirtualFileBrowser::GetFileList()
{
	vector<string> retVal;
	if(m_CurrentFolder.size()>=2)
	{
		string currentFolder = m_CurrentFolder.at(0);
		if(currentFolder == "Xbox360 Games" || currentFolder == "Xbox Live Arcade" || currentFolder == "Homebrew Games" || currentFolder == "Xbox Classic" || currentFolder == "Emulators")
		{
			
			string path  = GetRealCurrentPath();
			DebugMsg("VirtualFileBrowser","CurrentGamePath = %s",path.c_str());
			FileBrowser fbr;
			fbr.CD(path);
			vector<string> subFolders = fbr.GetFileList();
			for(int x=0; x < (int)subFolders.size();x++)
			{
				retVal.push_back(subFolders.at(x));
			}
				
		}
	}
	return retVal;
}