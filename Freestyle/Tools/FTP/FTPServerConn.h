#pragma once
#include "../Threads/cthread.h"
#include "../FileBrowser/FileBrowser.h"
#include "../FileBrowser/VirtualFileBrowser/VirtualFileBrowser.h"
#include <string>
#include <vector>
#include <sys/stat.h>

using namespace std;

// FTP Command tokens
typedef enum {
    USER, PASS, CWD,  PORT, 
    QUIT, PASV, ABOR, DELE,
    RMD,  XRMD, MKD,  XMKD,
    PWD,  XPWD, LIST, NLST, 
    SYST, TYPE, MODE, RETR, 
    STOR, REST, RNFR, RNTO,
    STAT, NOOP, MDTM, xSIZE, 
	EXEC, REBO, CDUP, RAM,
	FEAT, XCRC, MFMT, SCRN,
	SHTD,
    UNKNOWN_COMMAND, INVALID_COMMAND = -1
}CmdTypes;

struct command_list {
    char *command;
    CmdTypes CmdNum;
};

#define XFERSIZE 1024*1024

typedef struct {
    char *command;
    CmdTypes CmdNum;
}Lookup_t;

static const Lookup_t CommandLookup[] = {
	"USER", USER, "PASS", PASS, "CWD",  CWD,  "PORT", PORT, 
	"QUIT", QUIT, "PASV", PASV, "ABOR", ABOR, "DELE", DELE,
	"RMD",  RMD,  "XRMD", XRMD, "MKD",  MKD,  "XMKD", XMKD,
	"PWD",  PWD,  "XPWD", XPWD, "LIST", LIST, "NLST", NLST,  
	"SYST", SYST, "TYPE", TYPE, "MODE", MODE, "RETR", RETR,
	"STOR", STOR, "REST", REST, "RNFR", RNFR, "RNTO", RNTO,
	"STAT", STAT, "NOOP", NOOP, "MDTM", MDTM, "SIZE", xSIZE,
	"EXEC", EXEC, "REBO", REBO, "CDUP", CDUP, "RAM", RAM,
	"FEAT", FEAT, "XCRC", XCRC, "MFMT", MFMT, "SCREENSHOT", SCRN,
	"SHUTDOWN", SHTD
};

class CFTPServerConn :
	public CThread
{
private :
	FileBrowser* m_FileBrowser;
	bool easyMode;
	bool active;

	bool isLoggedIn;
	bool gotUser;

	bool CheckSkinFileOperation(string pFileName);

	string PathToFileSystemPath(string pPath);
public:
	bool isActive()
	{
		return active;
	}
    char XferBuffer[XFERSIZE];
	string GetPath(char* requested);
    struct sockaddr_in xfer_addr;
    BOOL PassiveMode;
    int PassiveSocket;
    int CommandSocket;
    int XferPort;
	string xboxip;
	
	CFTPServerConn(void);
	~CFTPServerConn(void);
	unsigned long Process(void* parameter);
	void SendReply(char * reply);
	CmdTypes GetCommand(char *CmdArg) ;
	string GetCurrentDir(void);
	
	void Cmd_NLST(string filename, bool Long, bool UseCtrlConn);

	int CreateTcpipSocket(int * Port);
	int ConnectTcpip(struct sockaddr_in *addr) ;
	void Send550Error();
	void SetEasyMode(bool value);
	
	void Cmd_XCRC(const char *filename);
	void Cmd_RETR(const char *filename);
	void Cmd_STOR(const char *filename);
	void Cmd_EXEC(const char *filename);
};
