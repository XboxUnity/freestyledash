#include "stdafx.h"

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "NTP.h"

unsigned long NTP::Process( void * parameter )
{
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	DebugMsg("NTP", "NTP Thread Work Begun");
	DoTimeSync();
	DebugMsg("NTP", "NTP Thread Work Complete");

	CloseHandle(this->hThread);
	this->hThread = 0;
	return 0;
}

void NTP::SyncTime()
{
	DebugMsg("NTP", "Starting Thread");
	bool retVal;
	retVal = CreateThread(CPU3_THREAD_1);
	if(retVal == true)
		DebugMsg("NTP", "Thread Created");
	else
		DebugMsg("NTP", "Failed To Create Thread");

}

void NTP::DoTimeSync()
{
	
	__int64 oldTime = 0;
	__int64 newTime = 0;
	
	UINT32 ret3= NtSetSystemTime(&newTime,&oldTime);
	ret3= NtSetSystemTime(&oldTime,&newTime);
	//Old time should now contain proper value
	byte NTPBuffer[48]={0};
	NTPBuffer[0] = 0x1b;
	//SET AT 40
	SYSTEMTIME LocalSysTime;
    GetSystemTime(&LocalSysTime);

	FILETIME ft;
	SystemTimeToFileTime(&LocalSysTime,&ft);
	
	__int64 ntpformat =0;
	memcpy(&ntpformat,&ft,8);

	byte* curTime = new byte[8];
	for(int x=0;x<8;x++)
	{
		curTime[x] =0;
	}
	memcpy(curTime,&ntpformat,8);
	DebugMsg("NTP","CurSysTime from 1600");
	DebugMsg("NTP","%02x %02x %02x %02x %02x %02x %02x %02x",curTime[0],curTime[1],curTime[2],curTime[3],curTime[4],curTime[5],curTime[6],curTime[7]);

	__int64 Diff = 0;
	byte* diffBytes = new byte[8];
	diffBytes[0] = 0x01;
	diffBytes[1] = 0x4f;
	diffBytes[2] = 0x37;
	diffBytes[3] = 0x3b;
	diffBytes[4] = 0xfd;
	diffBytes[5] = 0xe0;
	diffBytes[6] = 0x40;
	diffBytes[7] = 0x00;

	memcpy(&Diff,diffBytes,8);
	for(int x=0;x<8;x++)
	{
		curTime[x] =0;
	}
	memcpy(curTime,&Diff,8);
	DebugMsg("NTP","300 year is");
	DebugMsg("NTP","%02x %02x %02x %02x %02x %02x %02x %02x",curTime[0],curTime[1],curTime[2],curTime[3],curTime[4],curTime[5],curTime[6],curTime[7]);
	

	ntpformat-= Diff;
	for(int x=0;x<8;x++)
	{
		curTime[x] =0;
	}
	memcpy(curTime,&ntpformat,8);
	DebugMsg("NTP","CurSysTime from 1900");
	DebugMsg("NTP","%02x %02x %02x %02x %02x %02x %02x %02x",curTime[0],curTime[1],curTime[2],curTime[3],curTime[4],curTime[5],curTime[6],curTime[7]);
	for(int x=0;x<8;x++)
	{
		NTPBuffer[40+x] = curTime[x];
	}
	SOCKET debugSendSocket;
	SOCKADDR_IN debugSendSocketAddr;
	debugSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	BOOL bBroadcast = TRUE;


	if(setsockopt(debugSendSocket, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg("NTP","Failed to set debug send socket to 5801, error");
		return ;
	}

	

	debugSendSocketAddr.sin_family = AF_INET;
	debugSendSocketAddr.sin_family = AF_INET;

	XNDNS* pxndns = NULL;
	HANDLE myEvent = WSACreateEvent();
    XNetDnsLookup("pool.ntp.org", myEvent, &pxndns );
	DebugMsg("NTP","Waiting for dns...");
    WaitForSingleObject( myEvent, INFINITE );
    WSAResetEvent( myEvent );
    if( pxndns->iStatus != 0 )
    {
		DebugMsg("NTP","DNS ERROR");
		XNetDnsRelease( pxndns );
        shutdown( debugSendSocket, SD_BOTH );
		closesocket( debugSendSocket );
		return;
      
    }
	DebugMsg("NTP","DNS OK");
	debugSendSocketAddr.sin_addr.s_addr = inet_addr( "pool.ntp.org" );
    debugSendSocketAddr.sin_addr = pxndns->aina[ 0 ];
	debugSendSocketAddr.sin_port = 123;
	if( connect( debugSendSocket, ( struct sockaddr* )&debugSendSocketAddr, sizeof( debugSendSocketAddr ) ) == 0 )
	{
		DebugMsg("NTP","Connected");
		send( debugSendSocket,  (char*)NTPBuffer,48, 0 );

		recv( debugSendSocket,(char*) NTPBuffer, 48, 0 );
		byte* seconds = new byte[8];
		seconds[0] =0;
		seconds[1] = 0;
		seconds[2] = 0;
		seconds[3] = 0;
		seconds[4] = NTPBuffer[40];
		seconds[5] = NTPBuffer[41];
		seconds[6] = NTPBuffer[42];
		seconds[7] = NTPBuffer[43];

		
		__int64 secondsValue = 0;
		__int64 old = 0;
		memcpy(&secondsValue,seconds,8);
		secondsValue = 10000000 * secondsValue;
			
		memcpy(curTime,&secondsValue,8);
		DebugMsg("NTP","Final Set from 1900");
		DebugMsg("NTP","%02x %02x %02x %02x %02x %02x %02x %02x",curTime[0],curTime[1],curTime[2],curTime[3],curTime[4],curTime[5],curTime[6],curTime[7]);
		secondsValue = Diff +(secondsValue);
		curTime = new byte[8];
		for(int x=0;x<8;x++)
		{
			curTime[x] =0;
		}
		memcpy(curTime,&secondsValue,8);
		DebugMsg("NTP","Final Set from 1600");
		DebugMsg("NTP","%02x %02x %02x %02x %02x %02x %02x %02x",curTime[0],curTime[1],curTime[2],curTime[3],curTime[4],curTime[5],curTime[6],curTime[7]);
		UINT32 ret2= NtSetSystemTime(&secondsValue,&old);
		DebugMsg("NTP","SET RESULT : %08x",ret2);
	}
	else
	{
		DebugMsg("NTP","Cant Connect");
	}

	// Clean up pointer
	XNetDnsRelease( pxndns );
	shutdown( debugSendSocket, SD_BOTH );
	closesocket( debugSendSocket );
}