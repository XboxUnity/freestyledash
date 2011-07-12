//***********************************************************************************//
//*		The original code for this UDP Client came from Team XBMC and has been		*//
//*		modified and repurposed by Team FSD.  This code is subject to the same		*//
//*		GNU General Public License that the rest of FSD2.0 is licensed under.		*//
//*		Please give credit where credit is due if reusing this code.				*//
//***********************************************************************************//

#include "stdafx.h"
#include "UdpClient.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"

CUdpClient::CUdpClient( void )
{
	// Constructor for UDP Client
}

CUdpClient::~CUdpClient( void )
{
	// Deconstructor for UDP Client
	Stop();
}

bool CUdpClient::Create(void)
{
	
	InitializeCriticalSection(&critical_section);
	BOOL bBroadcast = TRUE;

	DebugMsg("UDPClient", "UDPCLIENT: Creating UDP socket...");

	// Create a UDP socket
	client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client_socket == SOCKET_ERROR) {
		DebugMsg("UDPClient", "UDPCLIENT: Unable to create socket.");
		return false;
	}
	
	// Patch Socket - Setting Socket Option 5802
	DebugMsg("UDPClient", "UDPCLIENT:  Setting Socket Option 5802");
	if ( setsockopt( client_socket, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0) {
		DebugMsg("UDPClient", "UDPCLIENT: Unable to set socket option 5802.");
		return false;
	}
	
	// Patch Socket - Setting Socket Option 5801
	DebugMsg("UDPClient", "UDPCLIENT:  Setting Socket Option 5801");
	if ( setsockopt( client_socket, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL)) != 0) {
		DebugMsg("UDPClient", "UDPCLIENT: Unable to set socket option 5801.");	
		return false;
	}

	// Set socket Broadcast option
	DebugMsg("UDPClient", "UDPCLIENT: Setting broadcast socket option...");
	if ( setsockopt( client_socket, SOL_SOCKET, SO_BROADCAST, (PCSTR)&bBroadcast, sizeof(BOOL) ) == SOCKET_ERROR) {
		DebugMsg("UDPClient", "UDPCLIENT: Unable to set socket option.");
		return false;
	}

	// Set non-blocking socket options
	DebugMsg("UDPClient", "UDPCLIENT: Setting non-blocking socket options...");
	BOOL bNonBlocking = TRUE;
	ioctlsocket(client_socket, FIONBIO, (unsigned long*)&bNonBlocking);

	// Spawn UDP Listener thread
	DebugMsg("UDPClient", "UDPCLIENT: Spawning listener thread...");
	Start();

	DebugMsg("UDPClient", "UDPCLIENT: Ready.");

	return true;
}

void CUdpClient::Destroy()
{
	Stop();
	Terminate();
	closesocket(client_socket);
	DeleteCriticalSection(&critical_section);
}

bool CUdpClient::Broadcast(int aPort, string aMessage)
{

	// Create a SOCKADDR_IN struct
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(aPort);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

	// Create a UdpCommand struct
	UdpCommand broadcast;
	broadcast.address = addr;
	broadcast.message = aMessage;
	broadcast.binary = NULL;
	broadcast.binarySize = 0;

	// Push udp command to vector for processsing
	EnterCriticalSection(&critical_section);
	commands.push_back(broadcast);
	LeaveCriticalSection(&critical_section);
	return true;
}


bool CUdpClient::Send(string aIpAddress, int aPort, string aMessage)
{
	// Create a SOCKADDR_IN struct
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(aPort);
	addr.sin_addr.s_addr = inet_addr(aIpAddress.c_str());

	// Create a UdpCommand struct
	UdpCommand transmit;
	transmit.address = addr;
	transmit.message = aMessage;
	transmit.binary = NULL;
	transmit.binarySize = 0;

	// Push udp command to vector for processsing
	EnterCriticalSection(&critical_section);
	commands.push_back(transmit);
	LeaveCriticalSection(&critical_section);
	return true;
}

bool CUdpClient::Send(SOCKADDR_IN aAddress, string aMessage)
{
	// Create a UdpCommand struct
	UdpCommand transmit;
	transmit.address = aAddress;
	transmit.message = aMessage;
	transmit.binary = NULL;
	transmit.binarySize = 0;

	// Push udp command to vector for processsing
	EnterCriticalSection(&critical_section);
	commands.push_back(transmit);
	LeaveCriticalSection(&critical_section);
	return true;
}

bool CUdpClient::Send(SOCKADDR_IN aAddress, LPBYTE pMessage, DWORD dwSize)
{
	// Create a UdpCommand struct
	UdpCommand transmit;
	transmit.address = aAddress;
	transmit.message = "";
	transmit.binary = pMessage;
	transmit.binarySize = dwSize;

	// Push udp command to vector for processsing
	EnterCriticalSection(&critical_section);
	commands.push_back(transmit);
	LeaveCriticalSection(&critical_section);
	return true;
}


unsigned long CUdpClient::Process(void* parameter)	
{
	// Set the threadname up for debugging
	SetThreadName("XlinkKai");
	DebugMsg("UDPClient", "UDPCLIENT: Listening.");

	// Local variables
	SOCKADDR_IN remoteAddress;
	char messageBuffer[2048];
	DWORD dataAvailable;

	while ( !m_bStop )
	{
		// Check and determine if there is any data available to read
		dataAvailable = 0;
		ioctlsocket(client_socket, FIONREAD, &dataAvailable);

		// While there is data to read
		while (dataAvailable > 0)
		{
			// read data
			int messageLength = sizeof(messageBuffer) - 1;
			int remoteAddressSize = sizeof(remoteAddress);

			int ret = recvfrom(client_socket, messageBuffer, messageLength, 0, (struct sockaddr *) & remoteAddress, &remoteAddressSize);
			if (ret != SOCKET_ERROR)
			{
				// Packet received
				messageLength = ret;
				messageBuffer[messageLength] = '\0';

				string message = messageBuffer;

				#ifdef _DEBUG
					DebugMsg("UDPClient", "UDPCLIENT RX: Message Recieved %s", message.c_str() );
				#endif
				// Forward the received message to the client message handler
				OnMessage(remoteAddress, message, (LPBYTE)messageBuffer, messageLength);
			}
			else
			{
				DebugMsg("UDPClient", "UDPCLIENT: Socket error %u", WSAGetLastError());
			}

			// is there any more data to read?
			dataAvailable = 0;
			ioctlsocket(client_socket, FIONREAD, &dataAvailable);
		}

			// dispatch a single command if any pending
			DispatchNextCommand();
	}

	// Clean up socket
	closesocket(client_socket);

	DebugMsg("UDPClient", "UDPCLIENT: Stopped listening.");
	return 0;
}

void CUdpClient::Start()
{
	if(!CreateThread(CPU1_THREAD_1))
	{
		DebugMsg("UDPClient","Failed to create thread");
	}	
	else
	{
		DebugMsg("UDPClient","Thread created");
	}
	m_bStop = false;
}

void CUdpClient::Stop()
{
	// Terminates the thread loop
	m_bStop = true;
}

void CUdpClient::DispatchNextCommand()
{
	EnterCriticalSection(&critical_section);

	if((int)commands.size() <= 0) {
		LeaveCriticalSection(&critical_section);
		// relinquish the remainder of this threads time slice
		Sleep(1);
		return;
	}

	COMMANDITERATOR it = commands.begin();
	UdpCommand command = *it;
	commands.erase(it);
	LeaveCriticalSection(&critical_section);

	int ret;
	if (command.binarySize > 0) {
		DebugMsg("UDPClient", "Sending Binary...");
		do {
			ret = sendto(client_socket, (LPCSTR) command.binary, command.binarySize, 0, (struct sockaddr *) & command.address, sizeof(command.address));
		} while (ret == -1);

		delete[] command.binary;
	}
	else
	{
		// only perform the following if logging level at debug
		//#ifdef _DEBUG
			DebugMsg("UDPClient", "Sending Message %s", command.message.c_str());
		//#endif

		do {
			ret = sendto(client_socket, command.message.c_str(), command.message.length(), 0, (struct sockaddr *) & command.address, sizeof(command.address));
		} while (ret == -1 && !true);
	}
}
