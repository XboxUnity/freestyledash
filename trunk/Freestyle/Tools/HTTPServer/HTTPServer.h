#pragma once
#include "../Threads/cthread.h"
#include "embedhttp.h"

void ApplyGameDetailTokens(ehttp &obj, DWORD dwContentId, DWORD dwContentType);
void ApplyGenericTokens( ehttp &obj);
void ApplyPathEntriesToken(ehttp &obj);
void ApplyGameScreenshotTokens(ehttp &obj, DWORD dwContentId, DWORD dwContentType);
string GenerateSessionId();

int handleGetRequest( ehttp &obj, void * cookie);
int handlePostRequest( ehttp &obj, void * cookie);

typedef struct _HTTP_SESSION_DATA {
	string SessionId;
	string Username;
	string Password;
} HTTP_SESSION_DATA;

class HTTPServer:public CThread
{
public:
	static HTTPServer& getInstance()
	{
		static HTTPServer singleton;
		return singleton;
	}

	bool IsExtensionBlocked(string szExtension);
	bool bXZPMode;

	string g_mActiveSessionId;
	string g_mGlobalSessionId;


	// Strings to hold the username and password of the user
	string m_szHttpUsername;
	string m_szHttpPassword;

private:
	
	vector<string> BlockedExtensions;

	void BuildExtensionList();
	
	
	unsigned long Process (void* parameter);	
	static const int m_ServerPort = 80;
	ehttp	http;
	void Start();
	void Stop();

	HTTPServer( );
	~HTTPServer();
	
	HTTPServer(const HTTPServer&);		            // Prevent copy-construction
	HTTPServer& operator=(const HTTPServer&);		    // Prevent assignment

};