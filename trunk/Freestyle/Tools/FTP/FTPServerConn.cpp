#include "stdafx.h"

#include "FTPServerConn.h"
#include "FTPFileWriter.h"
#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Generic/CRC_32.h"
#include "../Debug/Debug.h"
#include "../Managers/Drives/DrivesManager.h"
#include "../Managers/Drives/Drive.h"
#include "FTPServer.h"
#include "../Settings/Settings.h"
#include "../Managers/Skin/SkinManager.h"

CFTPServerConn::CFTPServerConn(void) : 
	easyMode(false), gotUser(false), isLoggedIn(false), PassiveMode(true)
{
}

CFTPServerConn::~CFTPServerConn(void)
{ 
}

void CFTPServerConn::SendReply(char * Reply)
{
    char ReplyStr[MAX_PATH+20];
    printf("    %s\n",Reply);
    sprintf_s(ReplyStr, MAX_PATH+20, "%s\r\n", Reply);
    send(CommandSocket, ReplyStr, strlen(ReplyStr),0);
}
void CFTPServerConn::SetEasyMode(bool value)
{
	easyMode = value;
	if(easyMode)
	
	{
		DebugMsg("FTPServerConn","New VirtualFileBrowser");
		m_FileBrowser = new VirtualFileBrowser();
	}
	else
	{
		DebugMsg("FTPServerConn","New FileBrowser");
		m_FileBrowser = new FileBrowser();
	}
}
unsigned long CFTPServerConn::Process(void* parameter)
{
	active = true;

    char buf[MAX_PATH+10];
    char repbuf[MAX_PATH+10];
 

	// Indicate ready to accept commands
    SendReply("220 FSD FTPD ready");

    PassiveSocket = CreateTcpipSocket(&XferPort);

    if (PassiveSocket < 0){
        closesocket(CommandSocket);
		closesocket(PassiveSocket);
		active = false;
		return 0;
    }

    if(listen(PassiveSocket, 1) == -1){
        FTPMsg("passive socket listen failed");
        closesocket(CommandSocket);
		closesocket(PassiveSocket);
		active = false;
		return 0;
    }
	string NewPath = "";


    for(;;)
	{

		if(SETTINGS::getInstance().getFtpServerOn() == false) {
			Sleep(500);
			continue;
		}
        // Get FTP command from input stream
        CmdTypes FtpCommand;
		
		DebugMsg("FTPServerConn", "buf: [[[[%s]]]]", buf);
        FtpCommand = GetCommand(buf);
		string commandString = buf;
		string curPath = "";
		switch(FtpCommand){
            case USER:
				
				DebugMsg("FTP","User %s %d %d",buf,strcmp(commandString.c_str(),"EasyUser"),commandString.length());
				if(strcmp(commandString.c_str(),"EasyUser") == 0)
				{
					gotUser = true;
					SetEasyMode(true);
					SendReply("331 Easy User login accepted");
				}
				else
				{
					SetEasyMode(false);
					if (commandString.compare(SETTINGS::getInstance().getFtpUser()) == 0) {
						gotUser = true;
						SendReply("331 User name okay, need password.");
					}
					else {
						SendReply("332 Need account for login.");
					}
				}
                
                break;

            case PASS:
				if (gotUser && commandString.compare(SETTINGS::getInstance().getFtpPass()) == 0) {
					SendReply("230 User logged in, proceed.");
					isLoggedIn = true;
				}
				else {
					SendReply("530 Incorrect Password");
					Sleep(2000);
				}

                break;

            case SYST:
                SendReply("215 Freestyle FTPd");
                break;
			case RAM:
				sprintf_s(repbuf,MAX_PATH+10, "250 (%4u total mb of ram free)",GetAvailableRam()/(1024*1024));
				SendReply(repbuf);
				break;
            case PASV:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                sprintf_s(repbuf,MAX_PATH+10, "227 Entering Passive Mode (%s,%d,%d)",xboxip.c_str(), XferPort >>8, XferPort & 0xff);
                for (int a=0;a<50;a++){
                    if (repbuf[a] == 0) break;
                    if (repbuf[a] == '.') repbuf[a] = ',';
                }
                SendReply(repbuf);
                PassiveMode = true; 
                break;

            case XPWD:
            case PWD: // Print working directory
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
				sprintf_s(repbuf,MAX_PATH+10,"257 \"%s\"", GetCurrentDir().c_str());
				SendReply(repbuf);
                break;
			case CDUP:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
				if (m_FileBrowser->IsAtRoot())
				{
					SendReply("550 CDUP command failed: already at root");
				}
				else
				{
					m_FileBrowser->UpDirectory();
					SendReply("250 CDUP command successful");
				}
				break;
            case NLST: // Request directory, names only.
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                Cmd_NLST(buf, false, FALSE);
                break;
            case LIST: 
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                Cmd_NLST(buf, false, FALSE);
                break;
            case STAT:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                Cmd_NLST(buf, false, TRUE);
                break;

            case DELE:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
			
				NewPath =GetPath(buf);
				
				//prevent skin file overwrite
				if (!CheckSkinFileOperation(NewPath))
				{
					SendReply("550 Permission denied");
					SendReply(" Workaround for compressed skin corruption");
					SendReply("	Select another skin first");
					SendReply("550 End");
					break;
				}


				if (unlink(NewPath.c_str())){
                    Send550Error();
                }else{
                    SendReply("250 DELE command successful.");
                }
                break;

            case RMD:
            case MKD:
            case XMKD:
            case XRMD:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                NewPath = GetPath(buf);

                if (FtpCommand == MKD || FtpCommand == XMKD){
					if (_mkdir(NewPath.c_str())){
                        Send550Error();
                    }else{
                        SendReply("257 Directory created");
                    }
                }else{
					DebugMsg("FtpServer","Delete %s",NewPath.c_str());
					if (RemoveDirectory(NewPath.c_str()) != 0){
						SendReply("250 RMD command successful");
                       
                    }else{
						DWORD err = GetLastError();
						DebugMsg("FtpServer","Error %08x",err);
                         Send550Error();
                    }
				}
                break;

            case RNFR:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                NewPath =GetPath(buf);
				//prevent skin file overwrite
				if (!CheckSkinFileOperation(NewPath))
				{
					SendReply("550 Permission denied");
					SendReply(" Workaround for compressed skin corruption");
					SendReply("	Select another skin first");
					SendReply("550 End");
					break;
				}

				if (FileExists(NewPath)){
			
                    SendReply("350 File Exists");
                }else{
                    SendReply("550 Path permission error");
                }
                break;
                
            case RNTO:
				{
					if (!isLoggedIn) {
						SendReply("530 Not logged in.");
						break;
					}
					// Must be immediately preceeded by RNFR!
					string rnToPath = GetPath(buf);

					//prevent skin file overwrite
					if (!CheckSkinFileOperation(rnToPath))
					{
						SendReply("550 Permission denied");
						SendReply(" Workaround for compressed skin corruption");
						SendReply("	Select another skin first");
						SendReply("550 End");
						break;
					}

					if (rename(NewPath.c_str(), rnToPath.c_str())){
					    Send550Error();
					}else{
					    SendReply("250 RNTO command successful");
					}
				}
                break;

            case ABOR:
                SendReply("226 Aborted");
                break;

			case MDTM: 				
				if (strlen(buf) == 0) {
					SendReply("501 Syntax error in parameters or arguments.");
				} else if (!isLoggedIn) {
					SendReply("530 Not logged in.");
				} else {
					NewPath = GetPath(buf);
					HANDLE hFile = ::CreateFile(NewPath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
					if (hFile == INVALID_HANDLE_VALUE) {
						sprintf_s(repbuf, "550 \"%s\": File not found.", NewPath.c_str());
						SendReply(repbuf);
					} else {
						FILETIME ft;
						SYSTEMTIME st;
						GetFileTime(hFile, 0, 0, &ft);
						CloseHandle(hFile);
						FileTimeToSystemTime(&ft, &st);
						sprintf_s(repbuf, "213 %04u%02u%02u%02u%02u%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
						SendReply(repbuf);
					}							
				}
				break;

			case MFMT:
				if (strlen(buf) == 0) {
					SendReply("501 Syntax error in parameters or arguments.");
				} else if (!isLoggedIn) {
					SendReply("530 Not logged in.");
				} else {
					int i;
					SYSTEMTIME st;
					st.wDayOfWeek = 0;
					st.wMilliseconds = 0;
					
					char* lpBuf = &buf[0];
		
					for (i = 0; i < 14; i++) {
						if ((buf[i] < '0') || (buf[i] > '9')) {
							break;
						}
					}
					if ((i == 14) && (buf[14] == ' ')) {
						strncpy_s(repbuf, buf, 4);
						repbuf[4] = 0;
						st.wYear = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 4, 2);
						repbuf[2] = 0;
						st.wMonth = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 6, 2);
						st.wDay = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 8, 2);
						st.wHour = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 10, 2);
						st.wMinute = (WORD)atoi(repbuf);
						strncpy_s(repbuf, buf + 12, 2);
						st.wSecond = (WORD)atoi(repbuf);
						lpBuf += 15;
					} else {
						SendReply("501 Syntax error in parameters or arguments.");
						break;
					}
					NewPath = GetPath(lpBuf);

					HANDLE hFile = CreateFileA(NewPath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0 );
					if (hFile == INVALID_HANDLE_VALUE) {
						sprintf_s(repbuf, "550 \"%s\": File not found.", NewPath.c_str());
						SendReply(repbuf);
					} else {
						FILETIME ft;
						SystemTimeToFileTime(&st, &ft);
						BOOL b = SetFileTime(hFile, 0, 0, &ft);
						DWORD gle = GetLastError();
						DebugMsg("FTP", "%d %d SYSTEMTIME %d %d %d %d %d %d", b, gle, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
						CloseHandle(hFile);
						SendReply("250 MFMT command successful.");
					}							
				}
				break;


            case CWD: // Change working directory
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
				DebugMsg("FTP","CWD");

				if (strcmp(buf, "..") == 0)
				{
					if (m_FileBrowser->IsAtRoot())
					{
						SendReply("550 CDUP command failed: already at root");
					}
					else
					{
						m_FileBrowser->UpDirectory();
						SendReply("250 CDUP command successful");
					}
					break;
				}


				NewPath = PathToFileSystemPath(buf);

				m_FileBrowser->CD(NewPath);
				
				if (NewPath.length() >= 1)
				{
					if (NewPath.substr(NewPath.length() - 1, 1) == "\\")
					{
						NewPath = NewPath.substr(0, NewPath.length() - 1);
					}
				}

				curPath = m_FileBrowser->GetCurrentPath();
				if (curPath.length() >= 1)
				{
					if (curPath.substr(curPath.length() - 1, 1) == "\\")
					{
						curPath = curPath.substr(0, curPath.length() - 1);
					}
				}

				if (stricmp(NewPath.c_str(), curPath.c_str()) == 0)
				{
					SendReply("250 OK");
				}
				else
				{
					sprintf_s(repbuf, "550 \"%s\": Path not found.", NewPath.c_str());
					SendReply(repbuf);
				}
                break;
 
            case TYPE: // Accept file TYPE commands, but ignore.
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                SendReply("200 Type set to I");
                break;

            case NOOP:
                SendReply("200 OK");
                break;

            case PORT: // Set the TCP/IP addres for transfers.
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}

                {
					DebugMsg("FTPServerConn", "PORT %s", buf);
                    int h1,h2,h3,h4,p1,p2;
                    char *a, *p;
                    sscanf_s(buf,"%d,%d,%d,%d,%d,%d",&h1,&h2,&h3,&h4,&p1,&p2);
					xfer_addr.sin_family = AF_INET;
                    a = (char *) &xfer_addr.sin_addr;
                    p = (char *) &xfer_addr.sin_port;
                    a[0] = (char)h1; a[1] = (char)h2; a[2] = (char)h3; a[3] = (char)h4;
                    p[0] = (char)p1; p[1] = (char)p2;
					PassiveMode = false;
                }
                SendReply("200 PORT command successful");
                break;
			case FEAT:
//				SendReply("211-Extension supported");
//				SendReply(" XCRC filename");
//				SendReply("211 End");
                SendReply("211-Extensions supported:\r\n XCRC filename\r\n SIZE\r\n REST STREAM\r\n MDTM\r\n MFMT\r\n TVFS\r\n211 END");

				break;
			case XCRC:	// not sure this should be protected by check for isLoggedIn, so not checking since it is harmless
				NewPath =GetPath(buf);
				Cmd_XCRC(NewPath.c_str());
				break;
            case RETR: // Retrieve File and send it
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
              
                /*if (NewPath == NULL){
                    SendReply(Conn, "550 Path permission error");
                    break;
                }*/
				Cmd_RETR(buf);
                break;

            case STOR: // Store the file.
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
             
				Cmd_STOR(buf);
                break;

            case UNKNOWN_COMMAND:
                SendReply("500 command not recognized");
                break;

            case QUIT: 
                SendReply("221 goodbye");
                closesocket(CommandSocket);
				closesocket(PassiveSocket);
				active = false;
				return 0;
			case EXEC:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
                //NewPath = TranslatePath(buf);
				Cmd_EXEC(buf);
				break;

			case SCRN:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}

				ScreenShot();
				SendReply("200 Screenshot taken.");
				break;
		
			case SHTD:
				if(!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
				SendReply("221 goodbye");
				closesocket(CommandSocket);
				closesocket(PassiveSocket);
				active = false;
				ShutdownXbox();
				break;
			case REBO:
				if (!isLoggedIn) {
					SendReply("530 Not logged in.");
					break;
				}
				SendReply("221 goodbye");
				closesocket(CommandSocket);
				closesocket(PassiveSocket);
				active = false;
				Restart();
				break;

            case INVALID_COMMAND:  // if GetCommand returns this, the connection is broken, so clean up
			    closesocket(CommandSocket);
				closesocket(PassiveSocket);
				active = false;
				return 0;  // and bail
				break;

            default: // Any command not implemented, return not recognized response.
                SendReply("500 command not implemented");
                break;
        }
	}
	/*
	active = false;

    //FTPMsg("Closing control connection\n");
    closesocket(CommandSocket);
	closesocket(PassiveSocket);
    //PortsUsed[Conn->XferPort & 255] = 0;
    //free(Conn);
		
	return 0;*/
}
string CFTPServerConn::GetPath(char* requested)
{
	vector<string> parts;
	StringSplit(requested,"/", &parts);
	if(parts.size()>1)
	{
		//got full path
		string retVal = "";
		for(unsigned int x=0;x<parts.size();x++)
		{
			if(x!=0)
			{
				retVal = retVal + "\\";
			}
			retVal = retVal + parts.at(x);
			if(x == 0)
			{
				retVal = retVal + ":";
			}
		}
		return retVal;
	}
	else
	{
		//only got file name
		return m_FileBrowser->GetCurrentPath() + "\\" + requested;
	}

	// can't reach this code
	/* if(requested[0] =='/')
	{
		//Got the complete path already
	}
	else 
	{
		//Only got a file name (or relative path?)
		
		return m_FileBrowser.GetCurrentPath() + "\\" + requested;
	} */
}

//------------------------------------------------------------------------------------
// Handle the NLST command (directory)
//------------------------------------------------------------------------------------
void CFTPServerConn::Cmd_NLST(string filename, bool Long, bool UseCtrlConn)
{
//   char repbuf[500];
    int xfer_sock;

	DebugMsg("FTP","Init Connection for LIST");
    if (UseCtrlConn){
        xfer_sock = CommandSocket;
    }else{
        SendReply("150 Opening connection");

        if (PassiveMode){
			DebugMsg("FTP","Passive Accept");
            xfer_sock = accept(PassiveSocket, NULL, NULL);
			DebugMsg("FTP","Accepted");
       }else{
            // Create TCP/IP connection for data link
			DebugMsg("FTP","active socket creation");
            xfer_sock = ConnectTcpip(&xfer_addr);
            if(xfer_sock < 0) {
                Send550Error();
                return;
            }
        }
    }

	bool ListAll = false;

  
	ListAll = true;
	Long = true;
	vector<string> folders = m_FileBrowser->GetFolderList();
	for(unsigned int x=0;x<folders.size();x++)
	{
		string out = "";
		string folder = folders.at(x);
		folder= str_replaceallA(folder,":","");
		if(Long)
		{
			out= m_FileBrowser->GetFolderFTPLongDescription(folder);
		}
		else
		{
			out= sprintfa("%s\r\n",folder.c_str());

		}
		send(xfer_sock, out.c_str(), strlen(out.c_str()),0);
		DebugMsg("FTPSERVERCONN", "PASSIVE MODE IS BROKEN AT ROOT- THIS DEBUG MESSAGE IS A HACK FIX-  PLEASE FIX ME.  KTHX");
	}
	vector<string> files = m_FileBrowser->GetFileList();
	string fullPath = m_FileBrowser->GetCurrentPath();
	for(unsigned int x=0;x<files.size();x++)
	{
		string out = "";
		string file = files.at(x);
	
		
		if(Long)
		{
			out= m_FileBrowser->GetFileFTPLongDescription(file);
		}
		else
		{
			out= sprintfa("%s\r\n",file.c_str());
		}
		send(xfer_sock, out.c_str(), strlen(out.c_str()),0);
	}
	//FTPMsg("Do TranslatePath %s",filename.c_str());
	//filename = Drive + DirNow + filename;
	
	/*if (Long){
					//struct tm tm;
					char DirAttr;
					char WriteAttr;

					// Call mktime to get weekday and such filled in.
					//tm = *localtime(&findFileData.ftLastWriteTime);
					//strftime(timestr, 20, "%b %d  %Y", &tm);
		     

					DirAttr = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 'd' : '-';
					WriteAttr = findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? '-' : 'w';

					sprintf_s(repbuf,500,"%cr%c-r%c-r%c-   1 root  root    %d %s %s\r\n", 
							DirAttr, WriteAttr, WriteAttr, WriteAttr,
							(unsigned)findFileData.nFileSizeLow,
							"Jan 20  2001",
							findFileData.cFileName); 
					//FTPMsg(repbuf);
				}else{
					sprintf_s(repbuf, 500, "%s\r\n",s.c_str());
				}
				send(xfer_sock, repbuf, strlen(repbuf),0);*/

	if (!UseCtrlConn){
        closesocket(xfer_sock);
        SendReply("226 Transfer Complete");
    }
}


CmdTypes CFTPServerConn::GetCommand(char *CmdArg) 
{
	DWORD dwMaxChars = 500;
    char InputString[500+1];
	char* psz = InputString;
    //int  CmdLen;
    char Command[6];
    int  a,b;

    DWORD dw, dwBytes;
    TIMEVAL tv;
    fd_set fds;
	tv.tv_sec=180;  // ftp connections only allow 180 seconds of idle
	tv.tv_usec=0;
	for (dwBytes=0;;dwBytes++) {
		FD_ZERO(&fds);
		FD_SET(CommandSocket,&fds);
		dw=select(0,&fds,0,0,&tv);
		if (dw==SOCKET_ERROR || dw==0) {
			DebugMsg("FtpServer", "Socket %d timed out", CommandSocket);
			SendReply("221 goodbye (connection timed out)");
			return INVALID_COMMAND; // Timeout
		}
		dw=recv(CommandSocket,psz,1,0);
		if (dw==SOCKET_ERROR || dw==0) {
			DebugMsg("FtpServer", "Socket %d got a network error, disconnected?", CommandSocket);
			return INVALID_COMMAND; // Network error
		}
		if (*psz=='\r') *psz=0;
		else if (*psz=='\n') {
			*psz=0;
			break;
			//return dwBytes;
		}
		if (dwBytes<dwMaxChars) psz++;
	}



    memset(Command, 0, sizeof(Command));
    for(a=0;a<5;a++) {
        if (!isalpha(InputString[a])) break;
        Command[a] = (char)toupper(InputString[a]);
    }

    b = 0;

    if (InputString[a++] == ' '){
        for (b=0;b<500-1;b++){
            if (InputString[a+b] < 32) break;
            CmdArg[b] = InputString[a+b];
        }
    }
    CmdArg[b] = 0;

   // FTPMsg("%s %s\n", Command, CmdArg);

    // Search through the list of known commands
    for(a=0;a<sizeof(CommandLookup)/sizeof(Lookup_t);a++){
        if(strncmp(Command, CommandLookup[a].command,4)==0){
            return CommandLookup[a].CmdNum;
        }
    }
   
    return UNKNOWN_COMMAND;
}

string CFTPServerConn::GetCurrentDir(void)
{
	string retVal = "\\";
	retVal = retVal + m_FileBrowser->GetCurrentPath();
	retVal= str_replaceallA(retVal,"\\","/");
	retVal= str_replaceallA(retVal,":","");
	return retVal;

}




int CFTPServerConn::CreateTcpipSocket(int * Port)
{
	SOCKET server;

	sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;
	local.sin_port=htons((u_short)*Port);

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

    // Get the assigned port number
	int size = sizeof(local);
    if (getsockname(server,(struct sockaddr *)&local,&size) < 0){
        FTPMsg("Error: getsockname() failed");
        return -1;
    }

    // Convert network byte order to host byte order
    *Port = ntohs(local.sin_port);

    return server;
}

int CFTPServerConn::ConnectTcpip(struct sockaddr_in *addr) 
{
	int addrlen = sizeof(struct sockaddr_in);
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0){
        FTPMsg("socket() failed");
        return -1;
    }

	// after setting these undocumented flags on a socket they should then run unencrypted
	BOOL bBroadcast = TRUE;

	if( setsockopt(sock, SOL_SOCKET, 0x5802, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		FTPMsg( "Failed to set socket to 5802, error");
		return 0;
	}

	if( setsockopt(sock, SOL_SOCKET, 0x5801, (PCSTR)&bBroadcast, sizeof(BOOL) ) != 0 )//PATCHED!
	{
		FTPMsg( "Failed to set socket to 5801, error");
		return 0;
	}

	if(connect(sock,(struct sockaddr *)addr, addrlen) < 0){
		int e = WSAGetLastError();
        FTPMsg("connect() failed [%d]", e);
        return -1;
    }
    return sock;
}

void CFTPServerConn::Send550Error()
{
    char ErrString[200];
#pragma warning(disable:4996)
    sprintf_s(ErrString, 200, "550 %s",sys_errlist[errno]);
#pragma warning(default:4996)
    SendReply(ErrString);
}

void CFTPServerConn::Cmd_XCRC(const char *filename) 
{
	FILE * fp = NULL;
	fopen_s(&fp, filename, "rb");
	if (!fp)
	{
        Send550Error();
        return;
    }
	CRC_32* crc32 = new CRC_32();
	DWORD crc = crc32->CalcCRC(fp);
	char crcString[200];
	sprintf_s(crcString, 200, "250 %08X", crc);
	SendReply(crcString);
	fclose(fp);
}

void CFTPServerConn::Cmd_RETR(const char *filename) 
{
    int xfer_sock;
    int size;

    // Check to see if the file can be opened for reading
	FILE * fp =  m_FileBrowser->OpenFile(filename);
    //if((file = open(filename, _O_RDONLY | _O_BINARY)) < 0) {
	if (!fp)
	{
        Send550Error();
        return;
    }

    // File opened succesfully, so make the connection
    SendReply("150 Opening BINARY mode data connection");

    if (PassiveMode){
        xfer_sock = accept(PassiveSocket, NULL, NULL);
    }else{
        // Create TCP/IP connection for data link
        xfer_sock = ConnectTcpip(&xfer_addr);
        if(xfer_sock < 0) {
            Send550Error();
            return;
        }
    }

    // Transfer file
    for(size=1;size > 0;){
		size = fread(XferBuffer, 1, XFERSIZE, fp);
		//FTPMsg("Got size %d",size);
        //size = read(file, XferBuffer, sizeof(XferBuffer));

        if(size < 0) {
            break;
        }

		int tosend = size;
		double sent = 1;
		int offset = 0;
		while (tosend > 0)
		{
        // Write buffer to socket.
			sent = send(xfer_sock, XferBuffer+offset, tosend, 0);
			CFTPServer::getInstance().AddBytes(sent,false);
			tosend -= (int)sent;
			offset += (int)sent;

			if (sent < 1)
			{
				FTPMsg("send failed");
				SendReply("426 Broken pipe") ;
				size = -1;
				tosend = 0;
			}
		}
    }
    

    if(size < 0){
        Send550Error();
    }else{
        SendReply("226 Transfer Complete");
    }

    closesocket(xfer_sock);
    fclose(fp);
}


bool CFTPServerConn::CheckSkinFileOperation(string pFileName)
{
	DebugMsg("FTPServerConn", "TODO: remove skin operation check!!!");
	return true;
	
	bool Result = true;
	string TmpSkinFile = SkinManager::getInstance().getCurrentFullSkinPath();
	pFileName = make_lowercaseA(pFileName);
	if(strcmp(TmpSkinFile.c_str(), pFileName.c_str()) == 0)
		Result = false;
	else
		Result = true;
	return Result;
}


void CFTPServerConn::Cmd_STOR(const char *filename) 
{

    int xfer_sock;
    int size;
	string FullPath = m_FileBrowser->GetWriteFilePath(filename);
	//prevent skin file overwrite
	if (!CheckSkinFileOperation(FullPath))
	{
		SendReply("550 Permission denied");
		SendReply(" Workaround for compressed skin corruption");
		SendReply("	Select another skin first, or use auto-updater");
		SendReply("550 End");
		return;
	}

	// Check to see if the file can be opened for writing
	FILE *file = NULL;
	fopen_s(&file,FullPath.c_str(), "wb");
	if(file == NULL){
		Send550Error();
		return;
	}
	fclose(file);

	SendReply("150 Opening BINARY mode data connection");

    if (PassiveMode){
        xfer_sock = accept(PassiveSocket, NULL, NULL);
    }else{
        // Create TCP/IP connection for data link
        xfer_sock = ConnectTcpip(&xfer_addr);
        if(xfer_sock < 0) {
            Send550Error();
            return;
        }
	}

	long Offset = 0;
	long Read = 0;
	long transferSize = XFERSIZE;
	string path(filename);
	if(path.find("Flash:") == 0)
	{
		transferSize = 0x4000;
		DebugMsg("FTPServerConn","Writingto flash");
	}

	FTPFileWriter wri(FullPath.c_str());

    // Transfer file
    for(size=1;size >= 0;){
        // Get from socket.
		while (size > 0 &&  Offset < transferSize)
		{
			size = recv(xfer_sock, XferBuffer + Offset, transferSize - Offset, 0);
			CFTPServer::getInstance().AddBytes(size,true);
			if(size < 0) {
				DebugMsg("FTPServerConn", "read failed (recv < 0)?");
			}
			Read += size;
			Offset += size;
		}

        if (Read <= 0) break;
	
		wri.SetBytes(((byte*)&XferBuffer),Read);
        // Write to file
/*       if (fwrite(XferBuffer, Read, 1, file) != 1){
			DebugMsg("FTPServerConn", "fwrite failed");
            size=-1;
            break;
        }
*/		 
		//Sleep(1);
		Read = 0;
		Offset = 0;
    }

    if(size < 0){
		DebugMsg("FTPServerConn","file save failed %s", filename);
        Send550Error();
    }else{
        SendReply("226 Transfer Complete");
    }

    closesocket(xfer_sock);
	wri.Close();
    // For some reason, the file ends up readonly - should fix that.
    //fclose(file);
}

string CFTPServerConn::PathToFileSystemPath(string pPath)
{
	string Result(pPath);

	//flip all slashes to backward slashes
	size_t Pos = 0;
	while ( (Pos = Result.find("/", Pos)) != Result.npos )
	{
        Result.replace( Pos, 1, "\\" );
        Pos++;
    }
	
	if (Result.length() > 1)
	{
		//make absolute path
		if (Result.find_first_of("\\", 0) == 0)
		{
			Result = Result.substr(1, Result.npos - 1);
		}
		else
		{
			if(!m_FileBrowser->IsAtRoot())
			{
				Result = m_FileBrowser->GetCurrentPath() + "\\" + Result;
			}
		}
		
		size_t InsertColonPos;
		if (Result.find_first_of(":") == Result.npos)
		{
			InsertColonPos = Result.find_first_of("\\", 0);
			if (InsertColonPos != Result.npos)
			{
				Result.insert(InsertColonPos, ":");
			}
			else
			{
				Result.append(":");
			}
		}
	}
	DebugMsg("FTPServerConn", "PathToFileSystemPath(string %s): %s", pPath.c_str(), Result.c_str());

	return Result;
}

void CFTPServerConn::Cmd_EXEC(const char *filename) 
{
	//MATTIE: fix automatic path from ftp command
	//i.e. "\hdd1\default.xex" becomes "hdd1:\default.xex"
	string RealPath(filename);
	size_t Pos = 0;
	size_t InsertColonPos = 0;
	
	
	while ( (Pos = RealPath.find("/", Pos)) != RealPath.npos )
	{
        RealPath.replace( Pos, 1, "\\" );
        Pos++;
    }
	
	if (RealPath.find_first_of("\\", 0) == 0)
	{
		RealPath = RealPath.substr(1, RealPath.npos - 1);
		InsertColonPos = RealPath.find_first_of("\\", 0);
		if (InsertColonPos != RealPath.npos)
		{
			RealPath.insert(InsertColonPos, ":");
		}
	}
	
	DebugMsg("FTPServerConn","FTP EXEC: filename: %s", filename);
	DebugMsg("FTPServerConn","FTP EXEC: realpath: %s", RealPath.c_str());

	if (FileExistsA(RealPath))
	{
		SendReply("221 goodbye");
		closesocket(CommandSocket);
		closesocket(PassiveSocket);

		XLaunchNewImage(RealPath.c_str(),0);
	} else {
		SendReply("550 File not found");
	}
}