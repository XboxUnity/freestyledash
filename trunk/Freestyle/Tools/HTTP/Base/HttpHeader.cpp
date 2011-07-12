#include "stdafx.h"
#include "HttpHeader.h"

HttpHeader::HttpHeader() {

}
HttpHeader::HttpHeader(const CHAR* Uri, const CHAR* RequestMethod, const CHAR* HttpVersion) {

	requestUri         = Uri;
	requestMethod      = RequestMethod;
	requestHttpVersion = HttpVersion;
}
HttpHeader::~HttpHeader(void) {

}
int HttpHeader::ParseResponceHeader(CHAR* buff, int len) {

	// Make sure its a valid http responce header
	if(len < 4) return -2;
	if(memcmp(buff, "HTTP", 4) != 0) return -2;

	// Loop and find our header end
	CHAR* curPos = buff; int size = 0;
	for(;;) {
		if((size + 4) > len) return -2;
		if(memcmp(curPos, "\r\n\r\n", 4) == 0) break;
		curPos++; size++;
	}
	DWORD headerLength = size + 4;

	// We got our header range, now lets make it a string
	string header(buff, 0, headerLength);

	// Break off our status line and parse it
	int statusEnd = header.find("\r\n");
	if(!ParseStatusLine(header.substr(0, statusEnd))) return -1;

	// Now we must break up our header fields
	int fieldsStart = statusEnd + 2;
	int fieldsSize = (headerLength - fieldsStart) - 4;
	if(!ParseHeaderFields(header.substr(fieldsStart, fieldsSize))) return -1;

	// Return our header length
	return headerLength;
}
BOOL HttpHeader::ParseStatusLine(string& StatusLine) {

	// Need to get our http version
	string::size_type verEnd = StatusLine.find_first_of(" ", 0);
	if(verEnd == string::npos) return FALSE;
	responceHttpVersion = StatusLine.substr(0, verEnd);
	
	// Get our status code
	string::size_type statusStart = verEnd + 1;
	string::size_type statusEnd   = StatusLine.find_first_of(" ", statusStart);
	if(statusEnd == string::npos) return FALSE;
    responceStatusCode = atoi(StatusLine.substr(
		statusStart, statusEnd - statusStart).c_str());

	// Now after that will be our status string
	string::size_type reasonStart = statusEnd + 1;
	string::size_type reasonEnd   = StatusLine.find_last_not_of(" \r\n");
	if(reasonEnd == string::npos) return FALSE;
	responceReasonPhrase = StatusLine.substr(
		reasonStart, (reasonEnd + 1) - reasonStart).c_str();

	return TRUE;
}
BOOL HttpHeader::ParseHeaderFields(string& HeaderFields) {

	// Clear all our current fields
	headerFieldEntries.clear();

    // Skip delimiters at beginning.
    string::size_type lastPos = HeaderFields.find_first_not_of("\r\n", 0);
    string::size_type pos     = HeaderFields.find_first_of("\r\n", lastPos);

	// Loop
    while (pos != string::npos || lastPos != string::npos) {

		// Found a value now parse it
		ParseHeaderField(HeaderFields.substr(lastPos, pos - lastPos));
		
		// Get our next item
        lastPos = HeaderFields.find_first_not_of("\r\n", pos);
        pos = HeaderFields.find_first_of("\r\n", lastPos);
    }

	return TRUE;
}
void HttpHeader::ParseHeaderField(string& Field) {

    string::size_type keyStart   = Field.find_first_not_of(" ", 0);
	string::size_type keyEnd     = Field.find_first_of(":", keyStart);

	string::size_type valueStart = Field.find_first_not_of(": ", keyEnd);
	string::size_type valueEnd   = Field.find_last_not_of(" ");

	AddHeaderField(
		Field.substr(keyStart, keyEnd).c_str(),
		Field.substr(valueStart, valueEnd + 1).c_str());
}
string& HttpHeader::GetHeaderField(const CHAR* Key) {

	// Make it a string and find it
	string key = Key;
	return headerFieldEntries[key];
}
DWORD HttpHeader::GetContentLength() {

	string length = GetHeaderField("Content-Length");
	if(length != "")
		return atoi(length.c_str());

	return 0;
}
void HttpHeader::AddHeaderField(const CHAR* Key, const CHAR* Value, ...) {

	// Format our value
	CHAR valueStr[1024];
	va_list ap;
	va_start(ap, Value);
	vsprintf_s(valueStr, sizeof(valueStr), Value, ap);
	va_end(ap);

	headerFieldEntries.insert(map<string, string>::value_type(
		Key, valueStr));
}
string HttpHeader::BuildRequestHeader() {

	// Build our request line
	string requestLine = requestMethod + " " + 
		requestUri + " " + requestHttpVersion + "\r\n";

	// Build our request headers now
	string requestFields = "";
	map<string, string>::iterator it;
	for(it = headerFieldEntries.begin(); it != headerFieldEntries.end(); it++) {
		string key   = (*it).first;
		string value = (*it).second;
		requestFields += key + ": " + value + "\r\n";
	}
	
	// Now build our full header
	string requestHeader = requestLine + requestFields + "\r\n";

	return requestHeader;
}