//--------------------------------------------------------------------------------------
// HttpClient.cpp
//
// XNA Developer Connection.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "HttpClient.h"

HttpClient::HttpClient(DWORD dwPort) {

    m_dwPort = dwPort;
	m_dwResponseCode = 0;

    // Create an event handle and setup an overlapped structure.
    m_overlapped.hEvent = WSACreateEvent();
    if( m_overlapped.hEvent == NULL )
        m_status = HTTP_STATUS_ERROR;
    else
        m_status = HTTP_STATUS_READY;
}
HttpClient::~HttpClient() {

    WSACloseEvent( m_overlapped.hEvent );
}
HRESULT HttpClient::Connect() {

	int nErrorCode;
	m_connected = FALSE;
	
	// Create TCP/IP socket
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if((m_socket == SOCKET_ERROR) || (m_socket == INVALID_SOCKET)) {
        nErrorCode = WSAGetLastError();
        SetSocketErrorCode(nErrorCode);
        SetStatus(HttpClient::HTTP_STATUS_ERROR_CONNECT);
        return nErrorCode;
    }

	// Patch our TCP/IP socket to run unencrypted
	BOOL sockOptValue = TRUE;
	if(setsockopt(m_socket, SOL_SOCKET, 0x5801, (PCSTR)&sockOptValue, sizeof(BOOL)) != 0) {
		nErrorCode = WSAGetLastError();
        SetSocketErrorCode(nErrorCode);
        SetStatus(HttpClient::HTTP_STATUS_ERROR_CONNECT);
        return nErrorCode;
	}

    sockaddr_in httpServerAdd;

    httpServerAdd.sin_family = AF_INET;
    httpServerAdd.sin_port = htons( m_dwPort );
    httpServerAdd.sin_addr.s_addr = inet_addr( m_serverName );

	// If we need to look up the host name
    if(httpServerAdd.sin_addr.s_addr == INADDR_NONE) {

        //DNS Lookup
		XNDNS* pxndns = NULL;
		XNetDnsLookup( m_serverName, m_overlapped.hEvent, &pxndns );
        WaitForSingleObject( m_overlapped.hEvent, INFINITE );
        WSAResetEvent( m_overlapped.hEvent );
        if(pxndns->iStatus != 0) {
            nErrorCode = pxndns->iStatus;
            XNetDnsRelease(pxndns);
            Disconnect();
            SetSocketErrorCode(nErrorCode);
            SetStatus(HttpClient::HTTP_STATUS_ERROR_CONNECT);
            return nErrorCode;
        }

        for(DWORD x = 0; x < pxndns->cina; ++x ) {
            httpServerAdd.sin_addr = pxndns->aina[x];
            if(connect(m_socket, (struct sockaddr*)&httpServerAdd, 
				sizeof(httpServerAdd)) == 0) {
				XNetDnsRelease(pxndns);
				m_connected = TRUE;
                return ERROR_SUCCESS;
            }
        }

        XNetDnsRelease(pxndns);
    } else {
        if(connect(m_socket, (struct sockaddr*)&httpServerAdd, 
			sizeof(httpServerAdd)) != 0) {
            nErrorCode = WSAGetLastError();
            Disconnect();
            SetSocketErrorCode(nErrorCode);
            SetStatus(HttpClient::HTTP_STATUS_ERROR_CONNECT);
            return nErrorCode;
		} else {
			m_connected = TRUE;		
			return ERROR_SUCCESS;
		}
    }

	return S_FALSE;
}
HRESULT HttpClient::Disconnect() {
	
	// Now go ahead and shutdown
    shutdown( m_socket, SD_BOTH );
    closesocket( m_socket );

	m_connected = FALSE;
	return ERROR_SUCCESS;
}
int HttpClient::Send(const char* buff, int len, int flags) {

	int ret = send(m_socket, buff, len, flags);

	if(ret == SOCKET_ERROR) {
		SetSocketErrorCode(WSAGetLastError());
		Disconnect();
	}

	return ret;
}
int HttpClient::Recieve(char* buff, int len, int flags) {

	int ret = recv(m_socket, buff, len, flags);

	if(ret == SOCKET_ERROR) {
		SetSocketErrorCode(WSAGetLastError());
		Disconnect();
	}

	return ret;
}
HRESULT HttpClient::SendCommand(HttpItem* httpItem, const char* RequestHeader, const char* RequestFooter) {

    // Connect first
	if(Connect() != ERROR_SUCCESS) return S_FALSE;

	// Handle our request
	DoSend(httpItem, RequestHeader, RequestFooter);

	// Handle our responce
	DoRecieve(httpItem, RequestHeader, RequestFooter);

	// Return success
	Disconnect();
    SetSocketErrorCode(ERROR_SUCCESS); 	
	return ERROR_SUCCESS;
}
HRESULT HttpClient::DoSend(HttpItem* httpItem, const char* RequestHeader, const char* RequestFooter) {
	
	// Send our request
	if(Send(RequestHeader, strlen(RequestHeader)) == SOCKET_ERROR) 
		return S_FALSE;

	// Now upload any data if we have any
	DWORD dataLength = httpItem->GetInputLength();
	int nSize = 0;
	CHAR buff[TCP_RECV_BUFFER_SIZE]; 
	if(dataLength > 0) {

		// Open our input
		httpItem->OpenInputData();

		// Send out our data
		while((nSize = httpItem->ReadInputData(
			(BYTE*)buff, TCP_RECV_BUFFER_SIZE)) != 0)
			if(Send(buff, nSize) == SOCKET_ERROR)
				return S_FALSE;

		// Close it out
		httpItem->CloseInputData();
	}

	// Send our footer if there is one
	if(RequestFooter != NULL) {
		if(Send(RequestFooter, strlen(RequestFooter)) == SOCKET_ERROR) 
			return S_FALSE;
	}

	// All done
	return S_OK;
}
HRESULT HttpClient::DoRecieve(HttpItem* httpItem, const char* RequestHeader, const char* RequestFooter) {

	// Recieve our header
	if(DoRecieveHeader() != S_OK)
		return S_FALSE;

	// Only if our responce is 200
	m_dwResponseCode = m_responceHeader.GetStatusCode();
	if(m_dwResponseCode != 200) {
		Disconnect();
		SetSocketErrorCode(HTTP_STATUS_ERROR);  
		return S_FALSE;
	}

	// Get some info our our responce data
	httpItem->outputBytesToDownload = m_responceHeader.GetContentLength();
	BOOL chunked = m_responceHeader.GetHeaderField("Transfer-Encoding") == "chunked";
	
	// Open our item to write to
	httpItem->OpenOutputData();	

	// Continue Recieve our data
	int nSize = 0; CHAR buff[TCP_RECV_BUFFER_SIZE];
	if(chunked) {

		// Our data is chunked and will have a size appended
		while((nSize = Recieve(buff, TCP_RECV_BUFFER_SIZE)) > 0)
			httpItem->WriteOutputData((BYTE*)buff, nSize);
	
	}else
		while((nSize = Recieve(buff, TCP_RECV_BUFFER_SIZE)) > 0)
			httpItem->WriteOutputData((BYTE*)buff, nSize);

	// Close our item
	httpItem->CloseOutputData();

	// We should be good
	return S_OK;
}
HRESULT HttpClient::DoRecieveHeader() {

	// Craete a memory buffer to store our data
	MemoryBuffer temp;

	// Recieve our header 1 char at a time
	CHAR buffer = 0;
	while(Recieve(&buffer, 1) == 1) {

		// Add our data and try and parse it
		temp.Add(&buffer, 1);
		int headerResult = m_responceHeader.ParseResponceHeader(
			(CHAR*)temp.GetData(), temp.GetDataLength());
		
		// Handle our parse responce
		if(headerResult == -2) continue;
		if(headerResult == -1) return S_FALSE;
		if(headerResult > 0) return S_OK;
	}
	
	// We had trouble downloading our data
	return S_FALSE;
}
HRESULT HttpClient::GET(HttpItem* Item) {
  
	// Create our request header with our fields
	HttpHeader header(Item->getPath().c_str(), "GET", "HTTP/1.1");
	header.AddHeaderField("Host", m_serverName);
	header.AddHeaderField("User-Agent", "Xbox Live Client/2.0.9199.0");
	header.AddHeaderField("Connection", "close");
	string requestString = header.BuildRequestHeader();

	// Now send our command
	return SendCommand(Item, requestString.c_str());
}

HRESULT HttpClient::POST(HttpItem* Item) {

	// Get our post variables string
	string postVarString = Item->GetPostVarString();

	// Create our request header with our fields
	HttpHeader header(Item->getPath().c_str(), "POST", "HTTP/1.0");
	header.AddHeaderField("Host", m_serverName);
	header.AddHeaderField("Content-Type", "application/x-www-form-urlencoded");
	header.AddHeaderField("Content-Length", "%d", postVarString.length());
	string requestString = header.BuildRequestHeader();

	// Now lets add the post vars to our request
	requestString += postVarString;

	// Now send our command
    return SendCommand(Item, requestString.c_str());
}
HRESULT HttpClient::POSTUpload(HttpItem* Item) {

	// Setup our boundary first, and create our post vars and file info
	string boundary = "----------xxxxxxxx";
	string requestHeader = Item->CreateMultiPartPostVars(boundary);
	requestHeader += Item->CreateMultiPartFileInfo(boundary);	

	// Setup our footer
	string requestFooter = Item->CreateMultiPartFooter(boundary);

	// Get the total size of all this data to send
	DWORD totalLength = requestHeader.length() + 
		Item->GetInputLength() + requestFooter.length();

	// Create our request header with our fields
	HttpHeader header(Item->getPath().c_str(), "POST", "HTTP/1.0");
	header.AddHeaderField("Host", m_serverName);
	header.AddHeaderField("Content-Type", 
		"multipart/form-data; boundary=%s", boundary.c_str());
	header.AddHeaderField("Content-Length", "%d", totalLength);
	string requestString = header.BuildRequestHeader();

	// Now lets add the post vars to our request
	requestString += requestHeader;

	// Now send our command
    return SendCommand(Item, requestString.c_str(), requestFooter.c_str());
}
HRESULT HttpClient::ExecuteItem(HttpItem* Item) {

	// Check our status first
    if(GetStatus() == HTTP_STATUS_BUSY) return E_FAIL;
    SetStatus(HttpClient::HTTP_STATUS_BUSY);

	// Set our server name
	strcpy_s(m_serverName, Item->getServer().c_str());

	DWORD tryCount = 0;
Restart:

	// Switch and handle our request method
	HRESULT result = S_FALSE;
	switch(Item->requestMethod) {
		case REQUEST_METHOD_GET:
			result = GET(Item);
			break;
		case REQUEST_METHOD_POST:
			result = POSTUpload(Item);
			break;
		default:
			return result;
	}

	// Check if it was moved
	if(result != ERROR_SUCCESS) {
		if(m_dwResponseCode == 302) {
			string serverName = m_responceHeader.GetHeaderField("Location");
			strcpy_s(m_serverName, serverName.c_str());
			OutputDebugString("302 Temporary Move, redirecting\r\n");
			goto Restart;
		} else if (m_dwResponseCode == 404) {
			return result;
		} else if (m_dwResponseCode == 0) {
			OutputDebugString("HttpClient:ExecuteItem got a response code of 0\r\n");
		} else {
			DebugBreak();
		}
	}

	// Return our result
	return result;
}