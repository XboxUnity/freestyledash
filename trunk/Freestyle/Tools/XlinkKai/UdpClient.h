//***********************************************************************************//
//*		The original code for this UDP Client came from Team XBMC and has been		*//
//*		modified and repurposed by Team FSD.  This code is subject to the same		*//
//*		GNU General Public License that the rest of FSD2.0 is licensed under.		*//
//*		Please give credit where credit is due if reusing this code.				*//
//***********************************************************************************//

#pragma once
#include "../Threads/cthread.h"
using namespace std;

class CUdpClient : public CThread
{
public:
	CUdpClient();
	virtual ~CUdpClient(void);

protected:
	unsigned long Process (void* parameter);
	void Start();
	void Stop();

	bool Create();
	void Destroy();

	bool Broadcast(int aPort, string aMessage);
	bool Send(string aIpAddress, int aPort, string aMessage);
	bool Send(SOCKADDR_IN aAddress, string aMessage);
	bool Send(SOCKADDR_IN aAddress, LPBYTE pMessage, DWORD dwSize);

	virtual void OnMessage(SOCKADDR_IN& aRemoteAddress, string aMessage, LPBYTE pMessage, DWORD dwMessageLength){};

	struct UdpCommand
	{
		SOCKADDR_IN address;
		string message;
		LPBYTE binary;
		DWORD binarySize;
	};

	void DispatchNextCommand();

	SOCKET client_socket;
	bool m_bStop;

	vector<UdpCommand> commands;
	typedef vector<UdpCommand> ::iterator COMMANDITERATOR;

	CRITICAL_SECTION critical_section;
};
