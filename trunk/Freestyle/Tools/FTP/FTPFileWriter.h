#pragma once
#include "../Threads/cthread.h"
#include "../Debug/Debug.h"
#include <string>
#include <vector>
#include <sys/stat.h>

using namespace std;


#define WRITINGPACKETMAXSIZE 1024*1024

	
class FTPFileWriter :
	public CThread
{
private :
	bool done;
	bool loopRunning;
	FILE* m_File;
	byte* currentBuffer;
	double currentBufferSize;
	bool waitingForBytes;
	
	unsigned long Process(void* parameter);
public:
	~FTPFileWriter(void){};
	FTPFileWriter(const char * path)
	{
		waitingForBytes = false;
		loopRunning = false;
		done = false;
	//	currentBuffer = (byte*)malloc(WRITINGPACKETMAXSIZE);
		fopen_s(&m_File,path, "wb");
		CreateThread(CPU3_THREAD_1);
	}
	void SetBytes(byte* buffer, double length)
	{
		if(length <10 || length > WRITINGPACKETMAXSIZE)
		{
			DebugMsg("FtpFileWriter","Trouble....");
		}
		while(!waitingForBytes)
		{
			Sleep(0);
		}
		currentBuffer = (byte*)malloc((size_t)length);
		memcpy(currentBuffer,buffer,(size_t)length);
		currentBufferSize = length;
		waitingForBytes = false;		
	}

	void Close()
	{	
		done = true;
		while(loopRunning == true)
		{
			Sleep(10);
		}
		fclose(m_File);
		Terminate();
	}
	
	

};
