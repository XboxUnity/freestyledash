#include "stdafx.h"

#include "Debug.h"
#include "../Generic/xboxtools.h"
//#include "smc.h"
#include "../Threads/cthread.h"
#include "./Commands/DebugCommand.h"
#include "./Commands/RestartCommand.h"
#include "./Commands/MemoryCommand.h"
#include "./Commands/VirtualKeyCommand.h"
#include "./Commands/ScreenShotCommand.h"
#include "./Commands/HudCommand.h"
#include "./Commands/BasicTestCommand.h"
#include "./Commands/TrayCloseCommand.h"
#include "./Commands/TrayOpenCommand.h"
#include "./Commands/DownloadCommand.h"
#include "./Commands/GetTimeCommand.h"
#include "./Commands/SetTimeCommand.h"
#include "./Commands/PingCommand.h"
#include "./Commands/LaunchXEXCommand.h"
#include "./Commands/ExtractCONCommand.h"
#include "./Commands/UnMountCONCommand.h"
#include "./Commands/HTTPPauseCommand.h"
#include "./Commands/HTTPResumeCommand.h"
#include "./Commands/AchievementCommand.h"
#include "./Commands/SqliteSnapshotCommand.h"
#include "./Commands/SqliteDeleteDB.h"
#include "./Commands/ConvertImagesToDXT5.h"
#include "./Commands/HTTPDownloadToMemory.h"
#include "./Commands/CheckSkinSetting.h"
#include "./Commands/ProfileCommand.h"
#include "./Commands/ExtractAvatarData.h"
#include "./Commands/LoadPluginCommand.h"
#include "./Commands/ToggleDebugDisplay.h"
#include "./Commands/FTPTestCommand.h"
#include "./Commands/FileBrowserCommand.h"
#include "./Commands/CoverCheck.h"
//#include "./Commands/SambaCommand.h"
// Kia Specific Commands
#include "./KaiCommands/KaiMessage.h"
#include "./KaiCommands/KaiConnect.h"
#include "./KaiCommands/KaiAppspec.h"
#include "./KaiCommands/KaiEndChat.h"
#include "./KaiCommands/KaiExitArena.h"
#include "./KaiCommands/KaiJoinArena.h"
#include "./KaiCommands/KaiPM.h"
#include "./KaiCommands/KaiShutdown.h"
#include "./KaiCommands/KaiStartChat.h"

#define KAI_SHOW_ALL

#include "../Generic/Tools.h"


using namespace std;

bool IsSending = false;
bool IsRecieving = false;
bool ToggleDebugTxt = false;

SOCKET debugSendSocket;
SOCKET debugRecieveSocket;
SOCKADDR_IN debugSendSocketAddr;
SOCKADDR_IN debugRecieveSocketAddr;


bool IsDebugMode()
{
	return ToggleDebugTxt;
}
void EnableDebugMode()
{
	IsSending = true;
	ToggleDebugTxt = true;
}
void SetDebugDisplay(bool toggle)
{
	ToggleDebugTxt = toggle;
}
class DebugReceiver:
	public CThread
{
protected :
	int m_port;

	typedef std::map<DebugCommand* const, DebugCommand* const> DCM;

public:
	DebugReceiver(void){

		//	DebugMsg("Command : %s",(sc).getCommandName().c_str());
	}
	string ip;
	~DebugReceiver(void)
	{
	}
	void SetCommands(std::vector<DebugCommand* const> _m_Commands)
	{


	}

	void SetPort(int port)
	{
		m_port = port;
	}
	unsigned long Process(void* parameter)
	{
		SetThreadName("DEBUGMSG");
		std::vector<DebugCommand* const> _m_Commands;
		DebugMsg("Debug","GotArray");
		XNADDR addr;
		XNetGetTitleXnAddr(&addr);
		char cip[16];
		sprintf_s(cip, 16, "%d.%d.%d.%d", (byte)addr.ina.S_un.S_un_b.s_b1,
			(byte)addr.ina.S_un.S_un_b.s_b2,
			(byte)addr.ina.S_un.S_un_b.s_b3,
			(byte)addr.ina.S_un.S_un_b.s_b4
			);
		ip = string(cip);

		_m_Commands.push_back(new RestartCommand());
		_m_Commands.push_back(new ScreenshotCommand());
		_m_Commands.push_back(new BasicTestCommand());
		_m_Commands.push_back(new VirtualKeyCommand());
		_m_Commands.push_back(new CoverCheckCommand());
#ifdef KAI_SHOW_ALL
//#ifdef USE_SQL
		_m_Commands.push_back(new SqliteSnapshotCommand());
		_m_Commands.push_back(new SqliteDeleteDB());
		_m_Commands.push_back(new MemoryCommand());
		_m_Commands.push_back(new ProfileCommand());
		_m_Commands.push_back(new LoadPluginCommand());
		_m_Commands.push_back(new ExtractAvatarDataCommand());
		_m_Commands.push_back(new CheckSkinXMLSetting());
		_m_Commands.push_back(new TrayCloseCommand());
		_m_Commands.push_back(new TrayOpenCommand());
		_m_Commands.push_back(new DownloadCommand());
		_m_Commands.push_back(new GetTimeCommand());
		_m_Commands.push_back(new SetTimeCommand());
		_m_Commands.push_back(new PingCommand());
		_m_Commands.push_back(new HTTPDownloadToMemoryCommand());
		_m_Commands.push_back(new LaunchXEXCommand());
		_m_Commands.push_back(new ExtractCONCommand());
		_m_Commands.push_back(new UnMountCONCommand());
		_m_Commands.push_back(new HTTPPauseCommand());
		_m_Commands.push_back(new HTTPResumeCommand());
		_m_Commands.push_back(new AchievementCommand());
		_m_Commands.push_back(new ConvertToDXT5());
		_m_Commands.push_back(new FTPTestCommand());
		_m_Commands.push_back(new HudCommand());
//		_m_Commands.push_back(new SambaCommand());
#endif
		_m_Commands.push_back(new ToggleDebugDisplay());
		_m_Commands.push_back(new KaiMessageCommand());
		_m_Commands.push_back(new KaiConnectCommand());
		_m_Commands.push_back(new KaiShutdownCommand());
		_m_Commands.push_back(new KaiPMCommand());
		_m_Commands.push_back(new KaiStartChatCommand());
		_m_Commands.push_back(new KaiEndChatCommand());
		_m_Commands.push_back(new KaiAppspecCommand());
		_m_Commands.push_back(new KaiJoinArenaCommand());
		_m_Commands.push_back(new KaiExitArenaCommand());
		_m_Commands.push_back(new FileBrowserCommand());

		DebugMsg("Debug","Command Set %d",_m_Commands.size());
		//Recieving code
		debugRecieveSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		debugRecieveSocketAddr.sin_family=AF_INET;
		debugRecieveSocketAddr.sin_addr.s_addr=INADDR_ANY;
		debugRecieveSocketAddr.sin_port=htons(m_port);

		BOOL bBroadcast = TRUE;
		if(setsockopt(debugRecieveSocket, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
		{
			DebugMsg("Debug","Failed to set debug recieve socket to 5802, error");
			return false;
		}

		if(setsockopt(debugRecieveSocket, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
		{
			DebugMsg("Debug","Failed to set debug recieve socket to 5801, error");
			return false;
		}
		bind(debugRecieveSocket, (const sockaddr*)( &debugRecieveSocketAddr ), sizeof( SOCKADDR_IN ) );
		ForcedDebugMsg("Debug","DEBUG IP : %s",ip.c_str());

		ForcedDebugMsg("Debug","DEBUG LISTENER READY");
		//FILL COMMAND VECTOR

		//	DebugMsg("Debugger initialised with %d commands",m_Commands.size());
		char buffer[1024];
		int iResult;
		do {
			memset(buffer, 0, sizeof(buffer));
			iResult = recv(debugRecieveSocket, (char*)(&buffer), sizeof(buffer), 0 );
			if( iResult != SOCKET_ERROR )
			{
				string command(buffer);
				string param = "";
				int srcpos = command.find(" ");
				if (srcpos > -1)
				{
					param = command.substr(srcpos+1);
					command = command.substr(0,srcpos);
				}
				//	DebugMsg("Received %s",command.c_str());
				//	DebugMsg("Parm %s",param.c_str());
				//	DebugMsg("Debugger got %d commands",m_Commands.size());
				if(strncmp(command.c_str(), "LIST",4)==0)
				{
					EnableDebugMode();
					DebugMsg("Debug","CommandList :",_m_Commands.size());
					for(unsigned int x=0;x<_m_Commands.size();x++)
					{
						DebugCommand* cmd = _m_Commands.at(x);
						DebugMsg("Debug","Command : %s",cmd->getCommandName().c_str());
					}
					/*		
					for(DCM::const_iterator it = _m_Commands.begin(); it != _m_Commands.end(); ++it)
					{
					DebugMsg("List a command...");
					//	string ccommand = it->first->getCommandName();
					//	DebugMsg("Command : %s",ccommand.c_str());
					}
					*/

				}
				else
				{
					bool found = false;
					for(unsigned int x=0;x<_m_Commands.size();x++)
					{
						DebugCommand* cmd = _m_Commands.at(x);
						string ccommand = cmd->getCommandName();
						if(strcmp(command.c_str(),ccommand.c_str())==0)
						{
							DebugMsg("Debug","Performing : %s",ccommand.c_str());

							cmd->Perform(param);

							found = true;
						}

					}
					/*	for(DCM::const_iterator it = _m_Commands.begin(); it != _m_Commands.end(); ++it)
					{
					string ccommand = it->first->getCommandName();

					if(strcmp(Command,ccommand.c_str())==0)
					{
					DebugMsg("Performing : %s",ccommand.c_str());
					it->first->Perform("");

					found = true;
					}

					}*/

					if(!found)
					{
						DebugMsg("Debug","%s : unknown command",buffer);
					}
				}
			}
		} while( true );

		return 0;
	}
};


DebugReceiver debugReceiver;

bool StartDebugger(int PortSend, int PortRecieve) {

	_unlink(sprintfa("%s%s", LOG_PATH, LOG_FILE).c_str());

	//create path if not exist
	RecursiveMkdir(sprintfa("%s", LOG_PATH));
	//Sender code

	debugSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	BOOL bBroadcast = TRUE;
	if(setsockopt(debugSendSocket, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg("Debug","Failed to set debug send socket to 5802, error");
		return false;
	}

	if(setsockopt(debugSendSocket, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		DebugMsg("Debug","Failed to set debug send socket to 5801, error");
		return false;
	}

	if(setsockopt(debugSendSocket, SOL_SOCKET, SO_BROADCAST, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )
	{
		DebugMsg("Debug","Failed to set debug send socket to SO_BROADCAST, error");
		return false;
	}

	debugSendSocketAddr.sin_family = AF_INET;
	debugSendSocketAddr.sin_addr.s_addr = INADDR_BROADCAST;
	debugSendSocketAddr.sin_port = htons(PortSend);
	//IsSending = true;

	//Start the thread

	debugReceiver.SetPort(PortRecieve);
	//EnableDebugMode();
	debugReceiver.CreateThread(CPU2_THREAD_1);
	IsRecieving = true;
	return true;
}

void EndDebugger() {
	if(IsSending) 
	{
		closesocket(debugSendSocket);
		IsSending = false;
	}
	if(IsRecieving) 
	{
		closesocket(debugRecieveSocket);
		IsRecieving = false;
	}
}
void DebugMsg(const WCHAR* filter, const WCHAR* message, ...)
{
	WCHAR buffer1[2048];
	WCHAR buffer2[2048];

	va_list ap;
	va_start (ap, message);
	vswprintf_s(buffer1, sizeof(buffer1), message, ap);
	va_end (ap);

	SYSTEMTIME st;
	GetLocalTime(&st);
	WCHAR time[20];
	//DWORD tid = GetCurrentThreadId();
	swprintf_s(time, 20, L"%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds); //, app.logLock.LockThreadId);

	wcscpy_s(buffer2, 2048, time);
	wcscat_s(buffer2, 2048, L"|");
	wcscat_s(buffer2, 2048, filter);
	wcscat_s(buffer2, 2048, L"|");
	DWORD tid = GetCurrentThreadId();
	WCHAR tids[20];
	swprintf_s(tids, 20, L"%X", tid);
	wcscat_s(buffer2, 2048, tids);
	wcscat_s(buffer2, 2048, L"|");
	wcscat_s(buffer2, 2048, buffer1);

	printf("%s\n", wstrtostr(buffer2));

	#ifndef _DEBUG
		wofstream writeLog;
		writeLog.open(sprintfa("%s%s", LOG_PATH, LOG_FILE),wofstream::app);
		if (writeLog.is_open())
		{
			writeLog.write(buffer2, wcslen(buffer2));
			writeLog.write(L"\n",1);
		}
		writeLog.close();
	#endif

	if(IsSending)
	{
		if (sendto(debugSendSocket, wstrtostr(buffer2).c_str(), wcslen(buffer2), 0, (const sockaddr*)(&debugSendSocketAddr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			DebugMsg("Debug","sendto error %d", WSAGetLastError());
	}
}
void DebugMsg(const char* filter, const char* message, ...)
{

	try
	{
	char buffer1[2048];
	char buffer2[2048];

	va_list ap;
	va_start (ap, message);
	vsprintf_s(buffer1, sizeof(buffer1), message, ap);
	va_end (ap);

	SYSTEMTIME st;
	GetLocalTime(&st);
	char time[20];
	//DWORD tid = GetCurrentThreadId();
	sprintf_s(time, 20, "%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds); //, app.logLock.LockThreadId);

	strcpy_s(buffer2, sizeof(buffer2), time);
	strcat_s(buffer2, sizeof(buffer2), "|");
	strcat_s(buffer2, sizeof(buffer2), filter);
	strcat_s(buffer2, sizeof(buffer2), "|");
	DWORD tid = GetCurrentThreadId();
	char tids[20];
	sprintf_s(tids, 20, "%X", tid);
	strcat_s(buffer2, sizeof(buffer2), tids);
	strcat_s(buffer2, sizeof(buffer2), "|");
	strcat_s(buffer2, sizeof(buffer2), buffer1);

	printf("%s\n", buffer2);

	#ifndef _DEBUG
		ofstream writeLog;
		writeLog.open(sprintfa("%s%s", LOG_PATH, LOG_FILE),ofstream::app);
		if (writeLog.is_open())
		{
			writeLog.write(buffer2, strlen(buffer2));
			writeLog.write("\n",1);
		}
		writeLog.close();
	#endif

	if(IsSending)
	{
		if (sendto(debugSendSocket, buffer2, strlen(buffer2), 0, (const sockaddr*)(&debugSendSocketAddr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			DebugMsg("Debug","sendto error %d", WSAGetLastError());
	}
	}
	catch(...)
	{
	}
}

void ForcedDebugMsg(const char* filter, const char* message, ...)
{

	char buffer1[2048];
	char buffer2[2048];

	va_list ap;
	va_start (ap, message);
	vsprintf_s(buffer1, sizeof(buffer1), message, ap);
	va_end (ap);

	SYSTEMTIME st;
	GetLocalTime(&st);
	char time[20];
	//DWORD tid = GetCurrentThreadId();
	sprintf_s(time, 20, "%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds); //, app.logLock.LockThreadId);

	strcpy_s(buffer2, sizeof(buffer2), time);
	strcat_s(buffer2, sizeof(buffer2), "|");
	strcat_s(buffer2, sizeof(buffer2), filter);
	strcat_s(buffer2, sizeof(buffer2), "|");
	DWORD tid = GetCurrentThreadId();
	char tids[20];
	sprintf_s(tids, 20, "%X", tid);
	strcat_s(buffer2, sizeof(buffer2), tids);
	strcat_s(buffer2, sizeof(buffer2), "|");
	strcat_s(buffer2, sizeof(buffer2), buffer1);
	
	#ifndef _DEBUG
		ofstream writeLog;
		writeLog.open(sprintfa("%s%s", LOG_PATH, LOG_FILE),ofstream::app);
		if (writeLog.is_open())
		{
			writeLog.write(buffer2, strlen(buffer2));
			writeLog.write("\n",1);
		}
		writeLog.close();
	#endif


	if (sendto(debugSendSocket, buffer2, strlen(buffer2), 0, (const sockaddr*)(&debugSendSocketAddr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		DebugMsg("Debug","sendto error %d", WSAGetLastError());
}

void DebugMsgRaw(const char* filter, const char* message)
{
	try
	{
	char buffer2[2048];

	SYSTEMTIME st;
	GetLocalTime(&st);
	char time[20];
	//DWORD tid = GetCurrentThreadId();
	sprintf_s(time, 20, "%02d:%02d:%02d.%03d",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds); //, app.logLock.LockThreadId);

	strcpy_s(buffer2, sizeof(buffer2), time);
	strcat_s(buffer2, sizeof(buffer2), "|");
	strcat_s(buffer2, sizeof(buffer2), filter);
	strcat_s(buffer2, sizeof(buffer2), "|");
	DWORD tid = GetCurrentThreadId();
	char tids[20];
	sprintf_s(tids, 20, "%X", tid);
	strcat_s(buffer2, sizeof(buffer2), tids);
	strcat_s(buffer2, sizeof(buffer2), "|");
	strcat_s(buffer2, sizeof(buffer2), message);

	printf("%s\n", buffer2);

	#ifndef _DEBUG
		ofstream writeLog;
		writeLog.open(sprintfa("%s%s", LOG_PATH, LOG_FILE),ofstream::app);
		if (writeLog.is_open())
		{
			writeLog.write(buffer2, strlen(buffer2));
			writeLog.write("\n",1);
		}
		writeLog.close();
	#endif

	if(IsSending)
	{
		if (sendto(debugSendSocket, buffer2, strlen(buffer2), 0, (const sockaddr*)(&debugSendSocketAddr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
			DebugMsg("Debug","sendto error %d", WSAGetLastError());
	}
	}
	catch(...)
	{
	}
}
