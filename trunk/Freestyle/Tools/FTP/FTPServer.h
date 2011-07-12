#pragma once
#include "../Threads/cthread.h"
#include "ftpserverconn.h"
#include <vector>
#include "../Managers/Timers/TimerManager.h"
using namespace std;

class CFTPServer :
	public CThread, public iTimerObserver
{
private:
	string m_CurrentStatus;
	double m_DownBytesTranfered;
	double m_UpBytesTranfered;

	CFTPServer();          // Private constructor
	~CFTPServer() {}
	CFTPServer(const CFTPServer&);                 // Prevent copy-construction
	CFTPServer& operator=(const CFTPServer&);      // Prevent assignment

	double getDownBytesLoaded()
	{
		double retVal = m_DownBytesTranfered;
		m_DownBytesTranfered = 0;

		return retVal;
	}
	double getUpBytesLoaded()
	{
		double retVal = m_UpBytesTranfered;
		m_UpBytesTranfered = 0;

		return retVal;
	}

public:

	string getStatus()
	{
		return m_CurrentStatus;
	}

	void AddBytes(double bytes,bool isDown)
	{
		if(isDown)
		{
			m_DownBytesTranfered = m_DownBytesTranfered + bytes;
		}
		else
		{
			m_UpBytesTranfered = m_UpBytesTranfered + bytes;
		}
	}
	void tick()
	{
		m_CurrentStatus = "";
		string downString = "";
		string upString = "";
		double lastSecsDownBytes = getDownBytesLoaded();
		if(lastSecsDownBytes != 0)
		{
			double kbps = lastSecsDownBytes/1024;
			if(kbps <1024)
			{
				downString = sprintfa("D : %.2f kBps",kbps);
			}
			else
			{
				double mbps = kbps/1024;
				downString = sprintfa("D : %.2f mBps",mbps);
			}
		}
		double lastSecsUpBytes = getUpBytesLoaded();
		if(lastSecsUpBytes != 0)
		{
			double kbps = lastSecsUpBytes/1024;
			if(kbps <1024)
			{
				upString = sprintfa("U : %.2f kBps",kbps);
			}
			else
			{
				double mbps = kbps/1024;
				upString = sprintfa("U : %.2f mBps",mbps);
			}
		}
		m_CurrentStatus = downString;
		if(m_CurrentStatus != "")
		{
			m_CurrentStatus = m_CurrentStatus + " - ";
		}
		m_CurrentStatus = m_CurrentStatus + upString;
		if(m_CurrentStatus == "")
		{
			m_CurrentStatus = "Connected";
		}

	}
	static CFTPServer& getInstance()
	{
		static CFTPServer singleton;
		return singleton;
	}
	bool HasActiveConnection();
	vector<CFTPServerConn*> Conns;
	char PortsUsed[256];
	int XferPort, XferPortStart, XferPortRange; 

	string xboxip;
	int port;
	//	string ftpuser;
	//	string ftppass;

	unsigned long Process(void* parameter);
	void TestSocket();
};
