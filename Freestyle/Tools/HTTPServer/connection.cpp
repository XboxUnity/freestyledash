/********************************************************************

	Connection Helper Class for Example Programs for:

	EhttpD - EasyHTTP Server/Parser C++ Class

	http://www.littletux.com

	Copyright (c) 2007, Barry Sprajc


	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS".  USE AT YOUR OWN RISK.

********************************************************************/
#include "stdafx.h"
#include "./connection.h"
#include "../debug/debug.h"
#include <winsockx.h>
Connection::Connection(int debugprint)
	{
	sd=connect_d=-1;
	debug=debugprint;
	}

Connection::~Connection()
	{
	}

void Connection::terminate( void )
	{
	if( sd!=-1)
		{
		close(sd);
		sd=-1;
		}
	}

int Connection::connect( char *ipAddress, unsigned short port )
{



	// Try to get a network socket.
	if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		DebugMsg( "HTTPServer-Connection","HTTPD:Can't create a socket\r\n");
		return -1;
		}
	BOOL bBroadcast = TRUE;
	if( setsockopt(sd, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg( "HTTPServer-Connection","Failed to set socket to 5802, error");
		return -1;
	}

	if( setsockopt(sd, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg( "HTTPServer-Connection","Failed to set socket to 5801, error");
		return -1;
	}

    memset(&sin, 0, sizeof(sin));

	// Create address we will bind to.
	
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	//bcopy(pH->h_addr, &sin.sin_addr, pH->h_length);
	int er=::connect(sd,(const sockaddr*)&sin,sizeof(sin));
	if( !er )
		return sd;
	return er;
}


int Connection::init( unsigned short port )
	{
	// Try to get a network socket.
	if ( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		DebugMsg( "HTTPServer-Connection","HTTPD:Can't create a socket\r\n");
		return -1;
		}

	BOOL bBroadcast = TRUE;
	if( setsockopt(sd, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg( "HTTPServer-Connection","Failed to set socket to 5802, error");
		return -1;
	}

	if( setsockopt(sd, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg( "HTTPServer-Connection","Failed to set socket to 5801, error");
		return -1;
	}
	// Create address we will bind to.
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port = htons(port);
	
   // Bind to the port
	int bnd=120;
	while( bnd>0 )
		{
		if (bind(sd, (const struct sockaddr *) &sin, sizeof(sin)) < 0)
			{
			DebugMsg( "HTTPServer-Connection","HTTPD:Can't bind....retrying.\r\n");
			Sleep(1);
			bnd--;
			}
		else bnd=0;
		}

	if( bnd<0 )
		{
		DebugMsg( "HTTPServer-Connection","HTTPD:Bind giving up\r\n");
		return -1;
		}
	
    DebugMsg( "HTTPServer-Connection","HTTPD:Bind OK\r\n");
 
	
	// Listen for the connection.
	if (listen(sd, 20) < 0)
		{
		DebugMsg( "HTTPServer-Connection","HTTPD:Can't listen to port\r\n");
		close(sd);
		return -1;
		}
	return 0;
	}


int Connection::accept( void )
	{

	int sock;
	static int linger[2]={0,0};
	//Accept connection.
	fromlen = sizeof(struct sockaddr);
	if ( (sock = ::accept(sd, (struct sockaddr *) &fsin, &fromlen)) < 0)
		{
		DebugMsg( "HTTPServer-Connection","HTTPD:Can't accept connection....\r\n");
		close(sd);
		sd=-1;
		return -1;
		}
	setsockopt(sock,SOL_SOCKET,SO_LINGER,(const char *)&linger,sizeof(linger));
	return sock;
	}

void Connection::close( int sock )
	{
	DebugMsg( "HTTPServer-Connection","Closing\r\n");
	closesocket(sock);
	DebugMsg( "HTTPServer-Connection","Done Closing\r\n");
	}



