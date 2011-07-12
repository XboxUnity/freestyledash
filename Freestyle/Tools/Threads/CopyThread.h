#pragma once
#include "cthread.h"

enum CopyStage {
	CS_IDLE,
	CS_CALCSIZE,
	CS_COPYING,
	CS_DONE,
	CS_CANCEL,
};

enum CD_ActionNames {
	CDA_COPYDVD,
	CDA_COPYFILES,
	CDA_MOVEFILES,
	CDA_SDMODE,
	CDA_DELETE,
};

class CCopyThread :
	public CThread
{
public:

    static CCopyThread& getInstance()
    {
        static CCopyThread singleton;
        return singleton;
    }

	CCopyThread(void);
	~CCopyThread(void);

	string Source;
	string Dest;

	CD_ActionNames Action;

	CopyStage Stage;
	LARGE_INTEGER Total_Size;
	LARGE_INTEGER Total_Prog;
	int Total_Files;
	int Total_FilesCopied;
	LARGE_INTEGER File_Size;
	LARGE_INTEGER File_Prog;
	bool Cancel;
protected:
	string CurFile;

public:
	unsigned long Process (void* parameter);
	void GetDirStat(string folder);
	void DoCopyDir(string source, string dest);
	void DoCopyFile(string source, string dest);
	void RemoveDirs(string source);


	string GetCurFile()
	{
		Lock();
		string File = CurFile;
		Unlock();
		return File;
	}
	void SetCurFile(string File)
	{
		Lock();
		CurFile = File;
		Unlock();
	}
};
