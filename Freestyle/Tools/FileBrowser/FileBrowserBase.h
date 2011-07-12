#pragma once

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"

class FileBrowserBase
{
protected:
	vector<string> currentDirectory;
	string currentRoot;
	FileBrowserBase() {};
public:
	virtual bool IsExistingDirectory(string directoryName) = 0;
	virtual bool IsExistingFile(string fileName) = 0;
	virtual bool GetDirectoryAndFileLists(vector<string>* directoryList, vector<string>* fileList) = 0;
	virtual string GetCurrentRealPath() = 0;
	string GetCurrentPath();
	bool ChangeDirectory(string newDirectory);
	bool UpToRoot();
	bool UpDirectory();
	bool IsAtRoot();
};

class SystemFileBrowser : public FileBrowserBase
{
public:
	SystemFileBrowser();
	bool IsExistingDirectory(string directoryName);
	bool IsExistingFile(string fileName);
	bool GetDirectoryAndFileLists(vector<string>* directoryList, vector<string>* fileList);
	string GetCurrentRealPath();
};

class ContentFileBrowser : public FileBrowserBase
{
public:
	ContentFileBrowser();
	bool IsExistingDirectory(string directoryName);
	bool IsExistingFile(string fileName);
	bool GetDirectoryAndFileLists(vector<string>* directoryList, vector<string>* fileList);
	string GetCurrentRealPath();
};

class FileBrowserFactory
{
public:
	enum FileBrowserType
	{
		System,
		Content
	};

private:
	FileBrowserBase* fileBrowser;
	FileBrowserType currentType;

public:
	FileBrowserFactory(FileBrowserType fbType = System);
	~FileBrowserFactory();

	FileBrowserBase* getFileBrowser();
	bool SwitchType(FileBrowserType fbType);
	FileBrowserType GetType();
};