#include "stdafx.h"
#include "CopyThread.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
CCopyThread::CCopyThread(void)
{
	Cancel = false;
	Stage = CS_IDLE;
}

CCopyThread::~CCopyThread(void)
{
}

void CCopyThread::GetDirStat(string folder)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = folder + "\\*.*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			GetDirStat(folder + "\\" + findFileData.cFileName);
		} else {
			Total_Files++;
			Total_Size.QuadPart += findFileData.nFileSizeLow;
		}
	} while (FindNextFile(hFind, &findFileData));

	FindClose(hFind);
}

unsigned long CCopyThread::Process(void* parameter)
{
    SetThreadPriority(hThread,THREAD_PRIORITY_HIGHEST);
    
	Stage = CS_CALCSIZE;

	//DebugMsg("Calc size of %s",Source.c_str());
	//DebugMsg("Last Char : %s",Source.substr(Source.size()-1,1).c_str());
	if (Source.substr(Source.size()-1,1) == "\\")
	{
		//DebugMsg("Scanning Dir");
		Total_Size.QuadPart = File_Size.QuadPart = 0;
		Total_Prog.QuadPart = File_Prog.QuadPart = 0;
		Total_FilesCopied = Total_Files = 0;

		GetDirStat(Source);
	} else {
		//DebugMsg("Scanning File");
		DWORD high = 0;
		DWORD size = 0;
		aGetFileSize(Source,size,high);

		Total_Size.QuadPart = File_Size.QuadPart = size;
		Total_Prog.QuadPart = File_Prog.QuadPart = 0;
		Total_Files = 1;
		Total_FilesCopied = 0;
	}

	//DebugMsg("Processing a total of %d bytes, %d files",Total_Size,Total_Files);

	Stage = CS_COPYING;

	if (Source.substr(Source.size()-1,1) == "\\")
	{
		//DebugMsg("Processing Dir");

		if (Dest.substr(Dest.size()-1,1) == "\\")
		{
			if (!FileExistsA(Dest))
				_mkdir(Dest.c_str());

			DoCopyDir(Source,Dest);
			if (Action == CDA_COPYDVD)
				DoCopyFile(Source + "default.xex",Dest + "default.xex");
		}
		
	} else {
		//DebugMsg("Processing File");
		
		// dest is a directory, do get filename from source and copy to that
		if (Dest.substr(Dest.size()-1,1) == "\\")
		{
			Dest += Source.substr(Source.rfind("\\")+1);
		}

		if (Action == CDA_DELETE)
		{
			_unlink(Source.c_str());
			Total_FilesCopied++;
		} else {
			DoCopyFile(Source,Dest);
		}
	}

	// fake file done prog bar, looks better
	File_Size.QuadPart = 1024;
	File_Prog.QuadPart = 1024;

	if (Cancel)
	{
		Stage = CS_CANCEL;
	} else {
		Stage = CS_DONE;
	}

    XamSetDvdSpindleSpeed(DVD_SPEED_2X);

	return 0;
}

void CCopyThread::DoCopyDir(string source, string dest)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = source + "\\*.*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		if (Action == CDA_MOVEFILES || Action == CDA_SDMODE || Action == CDA_DELETE)
		{
			source = str_replaceallA(source,"\\\\","\\");
			//DebugMsg("rmdir %s",source.c_str());
			int res = _rmdir(source.c_str());
			if (res != 0)
				DebugMsg("CopyThread","rmdir error %d - %d",res,GetLastError());
		}
		return;
	}

	do {
		if (Cancel)
		{
			FindClose(hFind);
			return;
		}

		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			string newdest = dest + "\\" + findFileData.cFileName;
			newdest = str_replaceallA(newdest,"\\\\","\\");
			if (Action != CDA_DELETE)
			{
				//DebugMsg("Make dir %s",newdest.c_str());
                /*if(Action == CDA_COPYDVD && newdest.find("$SystemUpdate") )
                {
                    continue;
                }*/
                _mkdir(newdest.c_str());
			}
			DoCopyDir(source + "\\" + findFileData.cFileName,newdest);
		} 
        else 
        {
			if (Action == CDA_COPYDVD && make_lowercaseA(string(findFileData.cFileName)) == "default.xex")
            {
				continue;
            }
            
            if (Action == CDA_DELETE)
			{
				string filename = source + "\\" + findFileData.cFileName;
				filename = str_replaceallA(filename,"\\\\","\\");
				//DebugMsg("unlink %s",filename.c_str());
				_unlink(filename.c_str());
				Total_FilesCopied++;
			} 
            else 
            {
				DoCopyFile(source + "\\" + findFileData.cFileName,dest + "\\" + findFileData.cFileName);
			}
		}
	} 
    while (FindNextFile(hFind, &findFileData));
	
    FindClose(hFind);

	if (Action == CDA_MOVEFILES || Action == CDA_SDMODE || Action == CDA_DELETE)
	{
		source = str_replaceallA(source,"\\\\","\\");
		//DebugMsg("rmdir %s",source.c_str());
		int res = _rmdir(source.c_str());
		if (res != 0)
			DebugMsg("CopyThread","rmdir error %d - %d",res,GetLastError());
	}
}

DWORD CALLBACK CopyProgressRoutine(
  LARGE_INTEGER TotalFileSize, // file size
  LARGE_INTEGER TotalBytesTransferred, // bytes transferred
  LARGE_INTEGER StreamSize, // bytes in stream
  LARGE_INTEGER StreamBytesTransferred, // bytes transferred for stream
  DWORD dwStreamNumber, // current stream
  DWORD dwCallbackReason, // callback reason
  HANDLE hSourceFile, // handle to source file
  HANDLE hDestinationFile, // handle to destination file
  LPVOID lpData // from CopyFileEx
)
{
	CCopyThread * ct = (CCopyThread*)lpData;

	ct->File_Size = TotalFileSize;
	ct->File_Prog = TotalBytesTransferred;

	if (ct->Cancel)
		return PROGRESS_CANCEL;

	return PROGRESS_CONTINUE;
}

void CCopyThread::DoCopyFile(string source, string dest)
{
	source = str_replaceallA(source,"\\\\","\\");
	dest = str_replaceallA(dest,"\\\\","\\");


	File_Size.QuadPart = File_Prog.QuadPart = 0;
	SetCurFile(source);

	if (Action == CDA_SDMODE)
	{
		//DebugMsg("Move %s to %s",source.c_str(),dest.c_str());
		if (MoveFile(source.c_str(),dest.c_str()))
			Total_FilesCopied++;
	} else {
		//DebugMsg("Copy %s to %s",source.c_str(),dest.c_str());
		if (CopyFileEx(source.c_str(),dest.c_str(),CopyProgressRoutine,(void*)this,NULL,0))
			Total_FilesCopied++;
	}

	if (Action == CDA_MOVEFILES)
	{
		//DebugMsg("_unlink %s",source.c_str());
		_unlink(source.c_str());
	}

	// add file progress to total progress and reset file stuff
	Total_Prog.QuadPart += File_Prog.QuadPart;
	File_Prog.QuadPart = 0;
	File_Size.QuadPart = 0;
}