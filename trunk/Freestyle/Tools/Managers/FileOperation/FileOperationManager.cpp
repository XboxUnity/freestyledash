#include "stdafx.h"

#include "FileOperationManager.h"
#include "FileOperation.h"
#include "../../FileBrowser/FileBrowser.h"
void FileOperationManager::AddFileOperation(string source, string destination, bool deleteSource)
{
	FileOperation op(source, destination,deleteSource);
	QueueFileOperation(op);
}

void FileOperationManager::AddVectorOperation(vector<FileInformation> Itm, string destination, bool deleteSource)
{
	for (int x = 0; x < Itm.size(); x++)
	{
		FileInformation itm = Itm.at(x);
		if (itm.isSelected)
		{
			string copysource;

			if (itm.path.substr(0,4).compare("smb:") == 0) {
				copysource = itm.path + "/" + itm.name;
			}
			else {
				copysource = itm.path + "\\" + itm.name;
			}

			string copysourcename = itm.name; 
			string copydestination = "";
			if (strcmp(destination.c_str(), "")!=0)
			{
				if (destination.substr(0,4).compare("smb:") == 0) {
					copydestination = destination + "/" + copysourcename;
				}
				else {
					copydestination = destination + "\\" + copysourcename;
				}

				//copydestination = destination + "\\" + copysourcename;
			}
			

			if (IsFolder(copysource))
			{
				FileOperationManager::getInstance().AddFolderOperation(copysource, copydestination, deleteSource);
			} else {
				FileOperationManager::getInstance().AddFileOperation(copysource, copydestination, deleteSource);
			}
		}
	}
}
void FileOperationManager::AddFolderOperation(string source, string destination, bool deleteSource)
{
	
	if (strcmp(destination.c_str(), "")!=0)
	{
		FileBrowser fSource;
		fSource.CD(source);

		FileBrowser fDest;
		fDest.CD(destination);

		vector<string> files =fSource.GetFileList();
		string sourceFolder;
		if (source.substr(0,4).compare("smb:") == 0) {
			sourceFolder = source + "/";
		}
		else {
			sourceFolder = source + "\\";
		}
		string destFolder;
		if (destination.substr(0,4).compare("smb:") == 0) {
			destFolder = destination + "/";
		}
		else {
			destFolder = destination + "\\";
		}
			
		for(unsigned int x=0;x<files.size();x++)
		{
			string sourceFile = sourceFolder + files.at(x);
			string destFile = destFolder + files.at(x);
			AddFileOperation(sourceFile,destFile,deleteSource);
		}
		vector<string> folders = fSource.GetFolderList();
	
		for(unsigned int x=0;x<folders.size();x++)
		{
			if(strcmp(folders.at(x).c_str(),"..") != 0)
			{
				string sourceFile = sourceFolder + folders.at(x);
				string destFile = destFolder + folders.at(x);
				AddFolderOperation(sourceFile,destFile,deleteSource);
			}
		}
		if (deleteSource)
			AddFileOperation(source, "", deleteSource);
	}else {
		FileBrowser fSource;
		fSource.CD(source);

		vector<string> files =fSource.GetFileList();
		string sourceFolder;
		
		if (fSource.GetCurrentPath().substr(0,4).compare("smb:") ==0 ) {
			sourceFolder = fSource.GetCurrentPath() + "/";
		}
		else {
			sourceFolder = fSource.GetCurrentPath() + "\\";
		}
		

		for(unsigned int x=0;x<files.size();x++)
		{
			string sourceFile = sourceFolder + files.at(x);
			AddFileOperation(sourceFile,"",deleteSource);
		}
		vector<string> folders = fSource.GetFolderList();
	
		for(unsigned int x=0;x<folders.size();x++)
		{
			if(strcmp(folders.at(x).c_str(),"..") != 0)
			{
				string sourceFile = sourceFolder + folders.at(x);
				AddFolderOperation(sourceFile,"",deleteSource);
			}
		}
		AddFileOperation(fSource.GetCurrentPath(), "", deleteSource);
	}
}
