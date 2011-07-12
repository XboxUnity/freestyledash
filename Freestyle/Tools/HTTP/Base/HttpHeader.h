#pragma once

#include "stdafx.h"
#include <string>
#include <vector>

using namespace std;

class HttpHeader
{
private:

	// Responce Header
	string responceHttpVersion;
	DWORD  responceStatusCode;
	string responceReasonPhrase;

	// Request Header
	string requestMethod;
	string requestUri;
	string requestHttpVersion; 

	// Header fields
	map<string, string> headerFieldEntries;

	// Responce Parsers
	BOOL ParseStatusLine(string& StatusLine);

	BOOL ParseHeaderFields(string& StatusLine);
	void ParseHeaderField(string& Field);	

public:

	// Responce methods
	DWORD   GetStatusCode()    { return responceStatusCode;   }
	string& GetReasponPhrase() { return responceReasonPhrase; }

	// Request methods
	void SetRequestMethod(const CHAR* Method)   { requestMethod      = Method; }
	void SetRequestUri(const CHAR* Uri)         { requestUri         = Uri;    }
	void SetRequestHttpVersion(const CHAR* Ver) { requestHttpVersion = Ver;    }

	string& GetRequestMethod()      { return requestMethod;      }
	string& GetRequestUri()         { return requestUri;         }
	string& GetRequestHttpVersion() { return requestHttpVersion; }

	// Shared methods
	void    AddHeaderField(const CHAR* Key, const CHAR* Value, ...);
	string& GetHeaderField(const CHAR* Key);

	// Constructor\Deconstructor
	HttpHeader();
	HttpHeader(const CHAR* Uri, const CHAR* RequestMethod = "GET", const CHAR* HttpVersion = "HTTP/1.0");
	~HttpHeader(void);

	// Public methods
	int ParseResponceHeader(CHAR* buff, int len);
	string BuildRequestHeader();

	DWORD GetContentLength();
};