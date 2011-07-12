#pragma once
#ifndef HttpClient_H
#define HttpClient_H

#include "stdafx.h"
#include "HttpHeader.h"
#include <winsockx.h>
#include "MemoryBuffer.h"
#include <string>
#include "../HttpItem.h"

using namespace std;

#define HTTP_HOST_IP_STRING_LENGTH  128
#define TCP_RECV_BUFFER_SIZE        512
#define HTTP_COMMAND_BUFFER_SIZE    512

class HttpClient {

public:

    enum HTTP_STATUS
    {
        HTTP_STATUS_READY,
        HTTP_STATUS_BUSY,
        HTTP_STATUS_DONE,
        HTTP_STATUS_ERROR,

		HTTP_STATUS_ERROR_CONNECT
    };

    HttpClient(DWORD dwPort = 80);
    ~HttpClient();

	HRESULT ExecuteItem(HttpItem* Item);
    HRESULT GET(HttpItem* Item);
    HRESULT POST(HttpItem* Item);
	HRESULT POSTUpload(HttpItem* Item);

    HTTP_STATUS GetStatus()                { return m_status;               }
    VOID  SetStatus(HTTP_STATUS status)    { m_status = status;             }
	VOID  SetSocketErrorCode(int ErrorCode){ m_socketErorrCode = ErrorCode; }
	DWORD GetResponseCode()				   { return m_dwResponseCode;       }

private:

	BOOL	      m_connected;
	SOCKET		  m_socket;
    HTTP_STATUS   m_status;
	DWORD		  m_dwPort;
	WSAOVERLAPPED m_overlapped;
	CHAR		  m_serverName[HTTP_HOST_IP_STRING_LENGTH];
	HttpHeader	  m_responceHeader;

	DWORD         m_dwResponseCode;
	int			  m_socketErorrCode;

	HRESULT Connect();
	HRESULT Disconnect();
	int Send(const char* buff, int len, int flags = 0);
	int Recieve(char* buff, int len, int flags = 0);

	HRESULT SendCommand(HttpItem* httpItem, const char* RequestHeader, const char* RequestFooter = NULL);
	HRESULT DoSend(HttpItem* httpItem, const char* RequestHeader, const char* RequestFooter);
	HRESULT DoRecieve(HttpItem* httpItem, const char* RequestHeader, const char* RequestFooter);
	HRESULT DoRecieveHeader();
};

#endif