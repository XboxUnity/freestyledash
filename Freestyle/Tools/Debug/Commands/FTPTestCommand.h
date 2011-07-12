#include "stdafx.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../FTPClient/ftplib.h"
#include <string>
#include <vector>
#include <fstream>

class FTPTestCommand:public DebugCommand
{
public :

	FTPTestCommand()
	{
		m_CommandName = "FTPTest";
	}

	void Perform(string parameters)
	{

		netbuf *conn;
		vector<string> info;
		XNDNS *m_pXNDns;
		struct sockaddr_in sin;
		string host;
		StringSplit(parameters, " ", &info);
			
		if ((sin.sin_addr.s_addr = inet_addr(info.at(0).c_str())) == -1)
		{
			HANDLE hEvent = CreateEvent(NULL, false, false, NULL);
			
			INT iErr = 0;
			iErr = XNetDnsLookup(
				info.at(0).c_str(),         // a string that prepresents the host name
				hEvent,          // the event handle from the CreateEvent call
				&m_pXNDns);      // the XNDNS structure to receive the IP info for the DNS entry found.
    
			while (m_pXNDns->iStatus == WSAEINPROGRESS) 
			{
				WaitForSingleObject(hEvent, INFINITE);
			}
			
			
			if (m_pXNDns->cina != 0)
			{
				char *buffer = NULL;
				XNetInAddrToString(m_pXNDns->aina[0], buffer, 32);
				//host = sprintfaA("%s", buffer);
				XNetDnsRelease(m_pXNDns);
				CloseHandle(hEvent);
				DebugMsg("FTPTest", "Connecting to %s", buffer);
				if (!FtpConnect(buffer,21, &conn))
				{
					DebugMsg("FTPTest", "Failed to open connection");
					return;
				}
			} else {
				DebugMsg("FTPTest", "Connecting to %s",info.at(0).c_str());
				CloseHandle(hEvent);
				if (!FtpConnect(info.at(0).c_str(),21, &conn))
				{
					DebugMsg("FTPTest", "Failed to open connection");
					return;
				}
			}
		} else {
			DebugMsg("FTPTest", "Connecting to %s", info.at(0).c_str());
			if (!FtpConnect(info.at(0).c_str(),21, &conn))
			{
				DebugMsg("FTPTest", "Failed to open connection");
				return;
			}
		}

		DebugMsg("FTPTest", "Logining in");
		if (!FtpLogin(info.at(1).c_str(), info.at(2).c_str(), conn))
		{
			DebugMsg("FTPTest", "Failed to login");
			FtpClose(conn);
			return;
		}

		if(FtpDir("game:\\othermedia.txt", "/", conn)) {
			fstream infile;
			infile.open("game:\\othermedia.txt", ios::in);
			if (infile.is_open()) {
				string buffer;
				//	get line from file
				while (getline(infile, buffer, '\n')) {
					DebugMsg("FTPTest", "file: %s", buffer.c_str());
				}
			}
			infile.close();
		} else {
			DebugMsg("FTPTest", "Failed to List");
			FtpClose(conn);
			return;
		}

		if(!FtpPut("game:\\othermedia.txt", "/othermedia.txt",FTPLIB_BINARY, conn))
		{
			DebugMsg("FTPTest", "Upload Failed");
			FtpClose(conn);
			return;
		}

		FtpClose(conn);
		return;
	}

};