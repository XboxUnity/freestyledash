#include "stdafx.h"

#include "FTPClientThread.h"
#include "../../Tools/FTPClient/ftplib.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/Managers/FileOperation/FileOperationManager.h"


bool FSDFtpC::CreateFSDFtpC( void )
{
	bool bResult = CreateThread( CPU2_THREAD_1 );

	return bResult;
}

void FSDFtpC::cancel()
{
	DebugMsg("FSDFtpC","Cancelling");
	//loop = false;
	//Terminate();
}

unsigned long FSDFtpC::Process (void* parameter)
{
	DebugMsg("FSDFtpC", "Starting process");
	netbuf *conn;

	XNDNS *m_pXNDns;
	struct sockaddr_in sin;
		
	if ((sin.sin_addr.s_addr = inet_addr(host.c_str())) == -1)
	{
		HANDLE hEvent = CreateEvent(NULL, false, false, NULL);
			
		INT iErr = 0;
		iErr = XNetDnsLookup(
			host.c_str(),         // a string that prepresents the host name
			hEvent,          // the event handle from the CreateEvent call
			&m_pXNDns);      // the XNDNS structure to receive the IP info for the DNS entry found.
    
		while (m_pXNDns->iStatus == WSAEINPROGRESS) 
		{
			WaitForSingleObject(hEvent, INFINITE);
		}
		
		unsigned int x = 0;
		if (m_pXNDns->cina != 0) {
			while (x <= m_pXNDns->cina)
			{
				char buffer[32];
				XNetInAddrToString(m_pXNDns->aina[0], buffer, 32);
				//host = sprintfaA("%s", buffer);
				XNetDnsRelease(m_pXNDns);
				CloseHandle(hEvent);
				DebugMsg("FTPTest", "Connecting to %s", buffer);
				if (FtpConnect(buffer,21, &conn))
				{
					x = m_pXNDns->cina;
				} else {
					x++;
				}
			}
		} else {
			DebugMsg("FTPTest", "Connecting to %s",host.c_str());
			CloseHandle(hEvent);
			if (!FtpConnect(host.c_str(),21, &conn))
			{
				DebugMsg("FTPTest", "Failed to open connection");
				return 1;
			}
		}
	} else {
		DebugMsg("FTPTest", "Connecting to %s", host.c_str());
		if (!FtpConnect(host.c_str(),21, &conn))
		{
			DebugMsg("FTPTest", "Failed to open connection");
			return 1;
		}
	}
	DebugMsg("FTPTest", "Logining in");
	if (!FtpLogin(username.c_str(), password.c_str(), conn))
	{
		DebugMsg("FTPTest", "Failed to login");
		FtpClose(conn);
		return 1;
	}

	if(!FtpPut(lpath.c_str(), rpath.c_str(), FTPLIB_BINARY, conn))
	{
		DebugMsg("FTPTest", "Upload Failed");
		FtpClose(conn);
		return 1;
	}

	FtpClose(conn);
	return 0;
}

void FSDFtpC::UploadFile( string s_Server , int d_port , string s_username , string s_password , string s_lpath , string s_rpath)
{
	host = s_Server;
	port = d_port;
	username = s_username;
	password = s_password;
	lpath = s_lpath;
	rpath = s_rpath;
}