#include "stdafx.h"

#include "FTPFileWriter.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Generic/CRC_32.h"
#include "../Debug/Debug.h"
#include "FTPServer.h"
unsigned long FTPFileWriter::Process(void* parameter)
{

	if(SETTINGS::getInstance().getFtpServerOn() == false) {
		Sleep(500);
		return 0;
	}

	SetThreadName("FtpFileWriter");
	loopRunning = true;
	waitingForBytes = true;
	while(!done || !waitingForBytes)
	{
		if(!waitingForBytes)
		{
		
			fwrite(currentBuffer,(size_t)currentBufferSize,1,m_File);
		
			delete currentBuffer;
			waitingForBytes = true;
		}
		else
		{
			Sleep(0);
		}
	}
	loopRunning = false;
	return 0;
}

