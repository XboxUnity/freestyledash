#include "stdafx.h"
#include "FTPServer.h"
#include "../Generic/xboxtools.h"
#include "../Settings/Settings.h"

CFTPServer::CFTPServer(void)
{
	m_CurrentStatus = "";
	m_DownBytesTranfered =0;
	m_UpBytesTranfered =0;

	TimerManager::getInstance().add(*this,1000);
	XNADDR addr;
	XNetGetTitleXnAddr(&addr);
	char ip[16];
	sprintf_s(ip, 16, "%d.%d.%d.%d", (byte)addr.ina.S_un.S_un_b.s_b1,
		(byte)addr.ina.S_un.S_un_b.s_b2,
		(byte)addr.ina.S_un.S_un_b.s_b3,
		(byte)addr.ina.S_un.S_un_b.s_b4
		);

	DebugMsg("FTPServer", "XNetGetTitleXnAddr returned %s",ip);
	string sIp = ip;


	xboxip = sIp; //cl.xboxip;	
	port = SETTINGS::getInstance().getPort(); 
	//	ftpuser = SETTINGS::getInstance().getFtpUser(); 
	//	ftppass = SETTINGS::getInstance().getFtpPass();
	CreateThread(CPU2_THREAD_1);
	
}



void CFTPServer::TestSocket()
{
}
bool CFTPServer::HasActiveConnection()
{
	bool retVal = false;
	for(unsigned int x=0;x<Conns.size();x++)
	{
		CFTPServerConn* conn = Conns[x];
		if(conn->isActive())
		{
			retVal = true;
			break;
		}
	}
	return retVal;
}
unsigned long CFTPServer::Process(void* parameter)
{
	SetThreadName("FTP Server");
	SOCKET server;

	sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;
	local.sin_port=htons((u_short)port);

    XferPortStart = XferPortRange = XferPort = 0;

	server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(server==INVALID_SOCKET)
	{
		FTPMsg( "INVALID SOCKET!");
		return 0;
	}

	// after setting these undocumented flags on a socket they should then run unencrypted
	BOOL bBroadcast = TRUE;

	if( setsockopt(server, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		FTPMsg( "Failed to set socket to 5802, error");
		return 0;
	}

	if( setsockopt(server, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		FTPMsg( "Failed to set socket to 5801, error");
		return 0;
	}

	if ( bind( server, (const sockaddr*)&local, sizeof(local) ) == SOCKET_ERROR )
	{
		int Error = WSAGetLastError();
		FTPMsg("bind error %d",Error);
		return 0; 
	}

	while ( listen( server, SOMAXCONN ) != SOCKET_ERROR )
	{

		if(SETTINGS::getInstance().getFtpServerOn() == false) {
			Sleep(500);
			continue;
		}

		SOCKET client;
		int length;

		length = sizeof(local);
		//FTPMsg("Trying accept");
		client = accept( server, (sockaddr*)&local, &length );

		CFTPServerConn* conn = new CFTPServerConn();
		conn->CommandSocket = client;
		DebugMsg("FTPServer","New Connection, XFERPORT : %d",XferPort);
		DebugMsg("FTPServer","Xbox Ip : %s",xboxip.c_str());
		conn->XferPort = XferPort;
		conn->xboxip = xboxip;
		DebugMsg("FTPServer","Xbox Ip : %s",conn->xboxip.c_str());
        PortsUsed[XferPort] = 1;

		Conns.push_back(conn);

		conn->CreateThread(CPU2_THREAD_1);
//		SetThreadPriority(conn->hThread,THREAD_PRIORITY_HIGHEST);

        // Cycle through port numbers.
        XferPort = XferPortStart;
        for(;XferPort < XferPortRange;XferPort++){
            // Find an unused port numbeer 
            // This code only relevant if a port range was specified.
            if (!PortsUsed[XferPort & 255]) break;
        }
	}

	return 0;
}