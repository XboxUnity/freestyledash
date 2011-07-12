/********************************************************************

EhttpD - EasyHTTP Server/Parser C++ Class

http://www.littletux.com

Copyright (c) 2007, Barry Sprajc

All rights reserved.

Redistribution and use in source and binary forms, with or without  
modification, are permitted provided that the following conditions  
are met:

Redistributions of source code must retain the above copyright 
notice, this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution.

Neither the name of Littletux.com nor the names of its contributors 
may be used to endorse or promote products derived from this 
software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL,EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE.

********************************************************************/



#include "stdafx.h"
#include "embedhttp.h"
#include "../Debug/Debug.h"
#include "../Generic/tools.h"
#include "../HTTP/Base/MemoryBuffer.h"
#include "../Generic/xboxtools.h"

size_t ehttpRecv(void *fd, void *buf, size_t len, void *cookie)
		{
		return recv((int)fd,(char*)buf,len,0);
		}

size_t ehttpSend(void *fd, const void *buf, size_t len, void *cookie)
		{
		return send((int)fd,(const char*)buf,len,0);
		}


int ehttp::getRequestType(void)
		{
		return requesttype;
		}

bool ehttp::isGetRequest(void)
		{
		return (requesttype==EHTTP_REQUEST_GET);
		}	

bool ehttp::isPostRequest(void)
		{
		return (requesttype==EHTTP_REQUEST_POST);
		}	
	

string & ehttp::getURL(void)
		{
		return url;
		}



string & ehttp::getFilename( void )
		{
		return filename;
		}

string ehttp::getUrlParam(char *key)
		{
		return url_parms[key];
		}

string ehttp::getPostParam(char *key)
		{
		return post_parms[key];
		}

map <string, string> & ehttp::getPostParams( void )
		{
		return post_parms;
		}

map <string, string> & ehttp::getUrlParams( void )
		{
		return url_parms;
		}

map <string, string> & ehttp::getRequestHeaders( void )
		{
		return request_header;
		}

string ehttp::getRequestHeader(char *key)
		{
		return request_header[key];
		}

void ehttp::out_replace_token( string tok, string val )
		{
		replace_token[tok]=val;
		}

void ehttp::out_set_file( char *fname, int ftype)
		{
		outfilename=fname;
		filetype=ftype;
		}

void ehttp::out_set_file(  string &fname, int ftype)
		{
		outfilename=fname;
		filetype=ftype;
		}	

void ehttp::out_set_file(  VOID * Buffer, DWORD Length, int ftype)
		{
		m_Buffer = Buffer;
		m_Length = Length;
		filetype=ftype;
		}

void ehttp::out_buffer_clear(void)
		{
		outbuffer="";
		}

void ehttp::out_write_str( char *str )
		{
		outbuffer+=str;
		}
		
void ehttp::out_write_str( string &str )
		{
		outbuffer+=str;
		}

int ehttp::out_replace_mem(void)
{

	// Setup our local variables
	CHAR* currentPos = (CHAR*)m_Buffer;
	DWORD sizeLeft = m_Length;

	int state=0;
	int err=0;
	int line=1;

	static char buffer[10240];
	string token;

	char c;

	if( filetype == EHTTP_MEMORY_FILE ) return EHTTP_MEMORY_FILE;

	if( m_Buffer == NULL )
	{
		err=-1;
		outbuffer="<html><BR>Cannot open the outfile from memory.<BR></html>";
	}
	// Loop while we still have data to send
	int status = 0;
	memset(buffer, 0, 10240);
	while(err == 0 && sizeLeft > 0 && m_Buffer != NULL) {

		// Send our data and break if we fail to send
		DWORD status = min(10240, sizeLeft);
		if(status < 0 ) return -1;

		memcpy(buffer, currentPos, status);

		for(int i = 0; i < (int)status; i++) {
			c = buffer[i];
			if( c=='\n' ) line++;
			switch( state )
			{
			// non token state 
			case 0:
				if( c == '#' ) state=1;
				else outbuffer+=c;
				break;

			// try to read the token start
			case 1:
				if( c == '#' )
				{ 
					token="";
					state=2;
				}
				else
				{
					outbuffer+='#';
					outbuffer+=c;
					state=0;
				}
				break;

			// read the token key name
			case 2:
				if( c == '#' ) state=3;
				else token+=c;
				break;
					
			// close of token name, replace the token
			case 3:
				if( c == '#' )
				{
					state=4;
					outbuffer+=replace_token[token];
					//DebugMsg( "HTTPServer-embed","Replacing token [%s] with [%s]\r\n",token.c_str(),(replace_token[token]).c_str());
					state=0;
				}
				else
				{
					DebugMsg( "HTTPServer-embed","(%d)Token Parse Error:%s \r\n",line,token.c_str());
					sizeLeft = 0;
					err=-2;
					state=99;
				}
				break;
										
			case 99:
				outbuffer+=c;
				break;
			}
		}

		// Decrement our size left
		currentPos += status;
		sizeLeft -= status;		
	}

	return EHTTP_ERR_OK;
}

int ehttp::out_replace(void)
		{
		int r;
		int state=0;
		int err=0;
		int line=1;

		static char buffer[10240];
		string token;

		char c;

		if( filetype == EHTTP_BINARY_FILE ) return EHTTP_BINARY_FILE;

		FILE *f=fopen(outfilename.c_str(),"r");
		
		if( !f )
			{
			err=-1;
			outbuffer="<BR>Cannot open the outfile <i>"+outfilename+"</i><BR>";
			}
		while( err== 0 && !feof(f) && f )
			{
			r=fread(buffer,1,sizeof(buffer),f);
			if( r < 0 ) return -1;
			// Read in the buffer and find tokens along the way
			for(int i=0;i<r;i++)
				{
				c=buffer[i];
				if( c=='\n' ) line++;

				
				switch( state )
					{
					// non token state 
					case 0:
						if( c == '#' ) state=1;
						else outbuffer+=c;
						break;

					// try to read the token start
					case 1:
						if( c == '#' )
							{ 
							token="";
							state=2;
							}
						else{
							outbuffer+='#';
							outbuffer+=c;
							state=0;
							}
						break;

					// read the token key name
					case 2:
						if( c == '#' ) state=3;
						else token+=c;
						break;
						
					// close of token name, replace the token
					case 3:
						if( c == '#' )
							{
							state=4;
							outbuffer+=replace_token[token];
							//DebugMsg( "HTTPServer-embed","Replacing token [%s] with [%s]\r\n",token.c_str(),(replace_token[token]).c_str());
							state=0;
							}
						else
							{
							DebugMsg( "HTTPServer-embed","(%d)Token Parse Error:%s \r\n",line,token.c_str());
							fseek(f,0,SEEK_END);
							err=-2;
							state=99;
							}
						break;
											
					case 99:
						outbuffer+=c;
						break;
				}
			}
		}
		if( f ) fclose(f);
		return EHTTP_ERR_OK;
	}

int ehttp::out_commit_memory(void)
{
	// Setup our local variables
	//if(!MmIsAddressValid((ULONGLONG)m_Buffer))
	//	return 0;

	CHAR* currentPos = (CHAR*)m_Buffer;
	DWORD sizeLeft = m_Length;

	// Loop while we still have data to send
	int status = 0;
	while(sizeLeft > 0) {

		// Send our data and break if we fail to send
		DWORD sendLen = min(1024 * 128, sizeLeft);
		if((status = pSend((void*)localFD, currentPos, sendLen, ptheCookie)) < 0)
			break;

		// Decrement our size left
		currentPos += status;
		sizeLeft -= status;		
	}

	if(m_Buffer) {
		free(m_Buffer);
		m_Length = 0;
	}

	
	// Return our result
	return status;
}
int ehttp::out_commit_binary(void)
		{
		int err=0;
		FILE *f=fopen(outfilename.c_str(),"rb");
		char buffer[1024*128];
		if(f)
		{
			while( !feof(f) )
			{
				int r=fread(buffer,1,1024*128,f);
				if( r>0 )
				{
					int remain=r;
					int total=remain;
					while( remain )
					{
						int w=pSend((void*)localFD,buffer+(total-remain),remain,ptheCookie);
						if( w<0 )
						{
							err=w;
							remain=0;

							fseek(f,0,SEEK_END);
							
						}
						else
						{
							remain-=w;
						}
					}
				}
			}
			fclose(f);
		}
			return err;
		}

int ehttp::out_commit(int header)
		{
		int w;
		int err=0;

		if( filetype == EHTTP_BINARY_FILE )
			return out_commit_binary();

		if( filetype == EHTTP_MEMORY_FILE )
			return out_commit_memory();

		if( header == EHTTP_HDR_OK )
			{
			string headr("HTTP/1.0 200 OK\r\n");
			map <string, string>::const_iterator iter;
			iter=response_header.begin();
			//Send out all the headers you want
			while( iter != response_header.end() )
				{
				headr+=iter->first+string(": ")+iter->second+string("\r\n");
				++iter;
				}
			outbuffer=headr+string("\r\n")+outbuffer;
			}
		else if( header == EHTTP_LENGTH_REQUIRED )
			{
			outbuffer=string("HTTP/1.0 411 Length Required\r\n\r\n");
			}
			

		int remain=outbuffer.length();
		int total=remain;
		while( remain )
			{
			w=pSend((void*)localFD,outbuffer.c_str()+(total-remain),remain,ptheCookie);
			if( w<0 )
				{
				err=w;
				remain=0;
				}
			else
				remain-=w;
			}
		return err;
		}

int ehttp::init( void )
	{
	pSend=ehttpSend;
	pRecv=ehttpRecv;
	pPreRequestHandler=NULL;
	return EHTTP_ERR_OK;
	}


void ehttp::add_handler( char *filename, int (*pHandler)(ehttp &obj, void *cookie))
	{
	if( !filename )
		pDefaultHandler=pHandler;
	else
		handler_map[filename]=pHandler;
	}

void ehttp::set_prerequest_handler( void (*pHandler)(ehttp &obj, void *cookie))
	{
		pPreRequestHandler=pHandler;
	}


int ehttp:: read_header( int fd, void *cookie, string &header, MemoryBuffer &message )
	{

	header="";
	unsigned int offset=0;
	int r = 0;
	while((offset=header.find("\r\n\r\n"))==string::npos )
		{
		input_buffer[0]=0;
		r=pRecv((void*)fd,&input_buffer[0],INPUT_BUFFER_SIZE,cookie);
		if( r < 0 )
			return EHTTP_ERR_GENERIC;
		input_buffer[r]=0;
		header+=input_buffer;
		}
	//message=header.substr(offset+4); // gotta have our shit globally i guess
	// gotta calculate somehow =/ hmmmm cuz r isnt good enough since some might be leftover
	// no cuz its teh offset in the full recieved header not our input buffer
	// input buffer is oging to change right we are interested in the last chunk of it only ahh
	DWORD inputPos = (offset + 4) % INPUT_BUFFER_SIZE;
	message.Add(&input_buffer[inputPos], r - inputPos);
	// then isntead of message add to our MemBuffer

	// can't we take the message from parse_message, and add it to our buffer if len > 0?
	// keep in mind that null will fuck us up =/ah so it would be pretty much this
	// and message= would be adding to mem buffer instead
	/* Fix the case where only "GET /xxxxx HTTP1.0\r\n\r\n" is sent (no other headers)*/
	if(offset == header.find("\r\n"))
		header=header.substr(0,offset+2);
	else
		header=header.substr(0,offset);
//	DebugMsg( "HTTPServer-embed","Header:-->%s<--\r\n",header.c_str());
//	DebugMsg( "HTTPServer-embed","Message:-->%s<--\r\n",message.c_str());
	return EHTTP_ERR_OK;
	}

int ehttp::parse_out_chunks( void *cookie, MemoryBuffer& memoryBuffer, vector<HTTP_CHUNK> &chunks)
{
	const BYTE * bufferAddr = (BYTE*)memoryBuffer.GetData();
	DWORD bufferSize = memoryBuffer.GetDataLength();

	// Get our boundary to search for
	string boundary = request_header["Content-Type"];
	boundary = "--" + boundary.substr(boundary.find("boundary=") + 9);

	// Now get a pointer to each chunk
	BYTE* temp = (BYTE*)bufferAddr;
	DWORD boundaryLen = boundary.length();
	DWORD foundChunkCount = 0;
	while((temp + boundaryLen) < (bufferAddr + bufferSize)) {
		
		if(memcmp(temp, boundary.c_str(), boundaryLen) == 0) {
			HTTP_CHUNK tempChunk;
			tempChunk.Data = temp;
			
			// Initialize the file data for this chunk
			tempChunk.FileData.Data = NULL;
			tempChunk.FileData.Length = 0;
			chunks.push_back(tempChunk);
			temp += boundaryLen;
			foundChunkCount++;
		} else
			temp++;
	}

	// Now we have our chunk positions, lets get our lengths
	vector<DWORD> sizes; DWORD size = 0;
	for(DWORD x = 0; x < foundChunkCount - 1; x++) {
		size = chunks.at(x + 1).Data - chunks.at(x).Data;
		//size =  chunks.at(x + 1) - chunks.at(x);
		chunks.at(x).Length = size;
	}
	size = (bufferAddr + bufferSize) - chunks.at(foundChunkCount - 1).Data;
	chunks.at(foundChunkCount - 1).Length = size;

	#ifdef DUMP_TO_FILE
	  if(!FileExists("game:\\HTTPDumps\\")) _mkdir("game:\\HTTPDumps\\");
	  if(!FileExists("game:\\HTTPDumps\\Chunks\\")) _mkdir("game:\\HTTPDumps\\Chunks\\");
	  for(int nCount = 0; nCount < (int)chunks.size(); nCount++) 
	  {
		  FILE * fHandle;
		  string fileName = sprintfaA("game:\\HTTPDumps\\Chunks\\ChunkDump_%d.bin", nCount);
		  fopen_s(&fHandle, fileName.c_str(), "wb");
		  fwrite(chunks.at(nCount).Data, 1, chunks.at(nCount).Length, fHandle);
		  fclose(fHandle);
	  }
	#endif

	// If it found chunks, return that it was successful
	return foundChunkCount == 0 ? EHTTP_ERR_GENERIC : EHTTP_ERR_OK;
}

int ehttp:: parse_out_entries( void *cookie, vector<HTTP_CHUNK> &chunk)
{
	string szCRLF = "\r\n";
	DWORD dwChunkCount = chunk.size();
	DWORD foundEntryCount = 0;
	DWORD totalEntryCount = 0;
	
	// Loop through our chunks and start finding entries
	for(unsigned int nCount = 0; nCount < dwChunkCount; nCount++) 
	{
		BYTE * chunkAddr = (BYTE*)chunk.at(nCount).Data;
		DWORD chunkSize = chunk.at(nCount).Length;

		BYTE * pos = (BYTE*)chunkAddr;
		BYTE * lastPos = chunkAddr;
		DWORD dwCRLFLen = szCRLF.length();

		// Reset our Entry Counter
		foundEntryCount = 0;
		// Walk through the chunk data and count entries, and store locations
		while((pos + dwCRLFLen) <= (chunkAddr + chunkSize)) {
			if(memcmp(pos, szCRLF.c_str(), dwCRLFLen) == 0 ) {
				foundEntryCount++;
				HTTP_CHUNK_ENTRY entry;
				entry.Data = lastPos;
				entry.Length = pos - lastPos;
				chunk.at(nCount).Entries.push_back(entry);
				pos += dwCRLFLen;
				if(lastPos == (pos - dwCRLFLen) ) {
					
					// Handle the data portion of the chunk
					HTTP_CHUNK_ENTRY entryData;
					entryData.Data = pos;
					entryData.Length = ((chunkAddr + chunkSize) - pos) - dwCRLFLen;
					foundEntryCount++;

					// Update our Chunk Struct
					chunk.at(nCount).Entries.push_back(entryData);
					chunk.at(nCount).FileData.Data = pos;
					chunk.at(nCount).FileData.Length = entryData.Length;
					break;
				}

				lastPos = pos;
			}
			else
				pos++;
		}

		

		// Set up our Entry Count
		chunk.at(nCount).EntryCount = foundEntryCount;
		totalEntryCount += foundEntryCount;

		#ifdef DUMP_TO_FILE
		  if(!FileExists("game:\\HTTPDumps\\")) _mkdir("game:\\HTTPDumps\\");
		  if(!FileExists("game:\\HTTPDumps\\Entries\\")) _mkdir("game:\\HTTPDumps\\Entries\\");
		  for(int i = 0; i < (int)chunk.at(nCount).EntryCount; i++) 
		  {
			  FILE * fHandle;
			  string fileName = sprintfaA("game:\\HTTPDumps\\Entries\\[Chunk_%d]EntryDump_%d.bin", nCount, i);
			  fopen_s(&fHandle, fileName.c_str(), "wb");
			  fwrite(chunk.at(nCount).Entries.at(i).Data, 1, chunk.at(nCount).Entries.at(i).Length, fHandle);
			  fclose(fHandle);
		  }

		  if(!FileExists("game:\\HTTPDumps\\")) _mkdir("game:\\HTTPDumps\\");
		  if(!FileExists("game:\\HTTPDumps\\FileData\\")) _mkdir("game:\\HTTPDumps\\FileData\\");
		  // If this chunk contains valid file data, let's write that out to file too
		  if(chunk.at(nCount).FileData.Length > 0) {
			  string dataFileName = sprintfaA("game:\\HTTPDumps\\FileData\\[Chunk_%d]FileData.bin", nCount);
			  FILE * fHandle;
			  fopen_s(&fHandle, dataFileName.c_str(), "wb");
			  fwrite(chunk.at(nCount).FileData.Data, 1, chunk.at(nCount).FileData.Length, fHandle);
			  fclose(fHandle);
		  }
		#endif
	}

	return totalEntryCount == 0 ? EHTTP_ERR_GENERIC : EHTTP_ERR_OK;
}

int ehttp:: parse_out_entry_headers( void *cookie, vector<HTTP_CHUNK> &chunk) 
{
	DWORD dwChunkCount = chunk.size();

	// Let's start looping through all our entries
	for( unsigned int nChunks = 0; nChunks < chunk.size(); nChunks++) {

		// Determine the entrycount for the current chunk
		DWORD dwEntryCount = chunk.at(nChunks).Entries.size();
		if( dwEntryCount == 0) return EHTTP_ERR_GENERIC;
		for( unsigned int nEntries = 0; nEntries < dwEntryCount; nEntries++) {
			
			// Calculate the Entry Type, and assign value from HTTP_ENTRYTYPE enum
			if(dwEntryCount > 1 && nEntries == dwEntryCount - 1) { 
				chunk.at(nChunks).Entries.at(nEntries).EntryType = HTTP_ENTRYTYPE_DATA;
				break;
			}
			if(nEntries == 0 && dwEntryCount == 1) {
				chunk.at(nChunks).Entries.at(nEntries).EntryType = HTTP_ENTRYTYPE_FOOTER;
			}
			else if(nEntries == 0 && dwEntryCount > 1) {
				chunk.at(nChunks).Entries.at(nEntries).EntryType = HTTP_ENTRYTYPE_BOUNDARY;
			}
			else if(chunk.at(nChunks).Entries.at(nEntries).Length == 0){
				chunk.at(nChunks).Entries.at(nEntries).EntryType = HTTP_ENTRYTYPE_BLANK;
			}
			else {
				chunk.at(nChunks).Entries.at(nEntries).EntryType = HTTP_ENTRYTYPE_HEADER;
			}

			DWORD EntryType = (DWORD)chunk.at(nChunks).Entries.at(nEntries).EntryType;

			BYTE * dwEntryAddr = chunk.at(nChunks).Entries.at(nEntries).Data;
			DWORD dwEntryLength = chunk.at(nChunks).Entries.at(nEntries).Length;

			char * szEntryString = (char*)malloc(dwEntryLength + 1 );
			memset(szEntryString, 0, dwEntryLength + 1);
			memcpy(szEntryString, dwEntryAddr, dwEntryLength);

			string strEntry = szEntryString;
			free(szEntryString);

			DebugMsg("HTTPServer-embed", "[ ENTRYTYPE: %d ] = %s", EntryType, strEntry.c_str());

			if(EntryType != HTTP_ENTRYTYPE_HEADER) {
				chunk.at(nChunks).Entries.at(nEntries).ContentKey = "";
				chunk.at(nChunks).Entries.at(nEntries).ContentValue = "";
				chunk.at(nChunks).Entries.at(nEntries).Parameters.clear();
				continue;
			}
			
			// Skip delimiters at beginning.
			string::size_type lastPos = strEntry.find_first_not_of("\r\n", 0);
			string::size_type pos     = strEntry.find_first_of("\r\n", lastPos);

			string strField = strEntry.substr(lastPos, pos - lastPos);
			
			string::size_type keyStart   = strField.find_first_not_of(" ", 0);
			string::size_type keyEnd     = strField.find_first_of(":", keyStart);

			string::size_type valueStart = strField.find_first_not_of(": ", keyEnd);
			string::size_type valueEnd   = strField.find_first_of(";");

			string key = "";
			string value = "";

			if(keyStart != string::npos)
				key = strField.substr(keyStart, keyEnd);

			if(valueStart != string::npos)
				value = strField.substr(valueStart, valueEnd - valueStart);

			if(strcmp(key.c_str(), "") != 0) {
				chunk.at(nChunks).Entries.at(nEntries).ContentKey = key;
				chunk.at(nChunks).Entries.at(nEntries).ContentValue = value;
				chunk.at(nChunks).Entries.at(nEntries).Parameters.clear();
			}

			// if the end of the contentValue is the end of the string, there are no additional parameters to parse, move on
			if(valueEnd == string::npos)
				continue;

			string strRemaining = strEntry.substr(valueEnd + 2);
			strRemaining = str_replaceallA(strRemaining, "; ", "&");
			DebugMsg("HTTPServer-embed", "Left to parse:  [%s]", strRemaining.c_str());

			parse_out_pairs(cookie, strRemaining, chunk.at(nChunks).Entries.at(nEntries).Parameters);
		}
	}

	return EHTTP_ERR_OK;
}

int ehttp:: parse_out_pairs( void *cookie, BYTE * entry, DWORD entrySize, map<string,string> &parms)
{

	return 0;
}

int ehttp:: parse_out_pairs( void *cookie, string &remainder, map <string, string> &parms )
	{
	string id;
	string value;
	int state=0;

	// run through the string and pick off the parms as we see them
	for(unsigned int i=0; i < remainder.length();)
		{
		switch( state )
			{
			case 0:
				id="";
				value="";
				state=1;
				break;

			case 1:
				switch( remainder[i] )
					{
					case '=':
						state=2;
						break;
					default:
						id+=remainder[i];
						break;
					}
				i++;
				break;

			case 2:
				switch( remainder[i] )
					{
					case '&':
						value = str_replaceallA(value, "\"", "");
						id = str_replaceallA(id, "\"", "");
						parms[id]=value;
						state=0;
						break;
					default:
						value+=remainder[i];
						break;
					}
				i++;
				break;
	
			}
		}
	// Add non-nil value to parm list
	if( state == 2 ) {
		value = str_replaceallA(value, "\"", "");
		id = str_replaceallA(id, "\"", "");
		parms[id]=value;
	}

	return EHTTP_ERR_OK;
	}


int ehttp:: parse_header( void *cookie, string &header )
{
	const char *request=NULL;
	const char *request_end=NULL;
	const char *pHeader=header.c_str();

	filename="";							
	contentlength=0;
	requesttype=-1;

	// Find end of request URL
DebugMsg( "HTTPServer-embed","pHeader == [%s]\r\n",pHeader);	
	request_end=strstr(pHeader," HTTP/1.0\r\n");
	if( !request_end )
			request_end=strstr(pHeader," HTTP/1.1\r\n");
	if( !request_end )
		{
		DebugMsg( "HTTPServer-embed","ERROR %d:%s\r\n",__LINE__,__FUNCTION__);
		return EHTTP_ERR_GENERIC;
		}

	// Is this a GET request
	if( requesttype == -1 )
		request=strstr(pHeader,"GET ");
	if( request )
		{
		request+=4;
		requesttype=EHTTP_REQUEST_GET;
		}

	// Is this a POST request
	if( requesttype == -1 )
		{
		request=strstr(pHeader,"POST ");
		if( request )
			{
			request+=5;
			requesttype=EHTTP_REQUEST_POST;
			}
		}

	// didn't find a get,post,etc...
	if( requesttype == -1 )
		{
		DebugMsg( "HTTPServer-embed","ERROR %d:%s\r\n",__LINE__,__FUNCTION__);
		return EHTTP_ERR_GENERIC;
		}

	// malformed request
	if( request_end <= request )
		{
		DebugMsg( "HTTPServer-embed","ERROR %d:%s\r\n",__LINE__,__FUNCTION__);
		return EHTTP_ERR_GENERIC;
		}

	// get the url requested
	while( request != request_end )
		{
		filename+=*request++;
		}

	// move to end of request
	request_end+=11;// length of " HTTP/1.1\r\n"
	
	// Save the complete URL
	url=filename;

	// See if there are params passed on the request
	int idx=filename.find("?");
	if( idx > -1 )
		{
		// Yank out filename minus parms which follow
		string remainder=filename.substr(idx+1);
		filename=filename.substr(0,idx);
		parse_out_pairs(cookie, remainder, url_parms);
		}

	// Find request headers,
	while( *request_end!='\r' && *request_end!='\0' )
		{
		const char *keyend=strstr(request_end,": ");

		// get the key
		if( keyend && keyend>request_end )
			{
			string key,value;
			key="";
			value="";
			while( keyend > request_end)
				key+=(*request_end++);

			//get the value of the key
			request_end=keyend+2;
			const char *valueend=strstr(request_end,"\r\n");

			//are we at end of header section
			if(!valueend)
				valueend=request_end+strlen(request_end);
			// read in the value
			if( valueend )
				{
				while( request_end < valueend )
					value+=*request_end++;
				//add key value pair to map
				request_header[key]=value;
				DebugMsg( "HTTPServer-embed","Header : ...%s... == ...%s...\r\n",key.c_str(),value.c_str());
				if( *request_end )
					request_end+=2;
				}
			}
		else
			{
			//Somthing went wrong
			DebugMsg( "HTTPServer-embed","ERROR %d:%s\r\n",__LINE__,__FUNCTION__);
			return EHTTP_ERR_GENERIC;
				}
		}
			
	// Find content length
	contentlength=atoi( request_header["CONTENT-LENGTH"].c_str() );
	if(contentlength == 0)
	{
		contentlength=atoi(request_header["Content-Length"].c_str() );
	}
	return EHTTP_ERR_OK;		
	

}

int ehttp:: parse_message( int fd, void *cookie, MemoryBuffer &message )
	{
	if( !contentlength ) return EHTTP_ERR_OK;

	DebugMsg( "HTTPServer-embed","Parsed content length:%d\r\n",contentlength);
	DebugMsg( "HTTPServer-embed","Actual message length read in:%d\r\n", message.GetDataLength());

	unsigned int recieved = message.GetDataLength();
	while( recieved < contentlength)
		{
		input_buffer[0]=0;
		int r = recv((int)fd, input_buffer, INPUT_BUFFER_SIZE, 0);
		if( r < 0 )
			return EHTTP_ERR_GENERIC;
		
		message.Add(input_buffer, r);
		
		DebugMsg( "HTTPServer-embed", "Message Length:  %d", r);
		
		// Decrement our recieved content
		recieved += r;

		}

	if(request_header["Content-Type"].find("multipart") == 0)
	{
		DebugMsg("HTTPServer-embed", "Parsing Multipart Upload");
		if(parse_out_chunks(cookie, message, post_chunks) == EHTTP_ERR_OK) {
			if(parse_out_entries(cookie, post_chunks) == EHTTP_ERR_OK) {
				if(parse_out_entry_headers( cookie, post_chunks) == EHTTP_ERR_OK) {
					DebugMsg("HTTPServer-embed", "Parsing Successful");
				}
			}
		}
	}
	else
	{

		// Got here, good, we got the entire reported msg length
		//DebugMsg( "HTTPServer-embed","Entire message is <%s>\r\n",message.c_str());
		string test = (char*)message.GetData();
		parse_out_pairs(cookie, test, post_parms);
	}

	

	return EHTTP_ERR_OK;
	}


int ehttp:: parse_request( int fd, void *cookie )
	{
	int (*pHandler)(ehttp &obj, void *cookie)=NULL;

	/* Things in the object which must be reset for each request */
	filename="";
	localFD=fd;
	ptheCookie=cookie;
	filetype=EHTTP_TEXT_FILE;
	url_parms.clear();
	post_parms.clear();
	multiform_chunks.clear();
	request_header.clear();
	replace_token.clear();
	contentlength=0;

	// Loop Through and free/clear our post_chunks
	DWORD chunkCount = post_chunks.size();
	for(int i = 0; i < (int)chunkCount; i++) {
		DWORD entryCount = post_chunks.at(i).Entries.size();
		for(int x = 0; x < (int)entryCount; x++) {
			post_chunks.at(i).Entries.at(x).Parameters.clear();
		}
		post_chunks.at(i).Entries.clear();
	}
	post_chunks.clear();


	string header;
	//string message;
	MemoryBuffer memBuffer;

	if( read_header(fd,cookie, header, memBuffer) == EHTTP_ERR_OK )
		if( parse_header(cookie,header)  == EHTTP_ERR_OK )
			if( parse_message(fd,cookie, memBuffer)  == EHTTP_ERR_OK )
				{

				// We are HTTP1.0 and need the content len to be valid
				// Opera Broswer
				if( contentlength==0 && requesttype==EHTTP_REQUEST_POST )
					{
					return out_commit(EHTTP_LENGTH_REQUIRED);
					}

				
				//Call the default handler if we didn't get the filename
				out_buffer_clear();

				if( pPreRequestHandler ) pPreRequestHandler( *this, ptheCookie );
				if( !filename.length() )
					{
					DebugMsg( "HTTPServer-embed","%d Call default handler no filename \r\n",__LINE__);
					return pDefaultHandler( *this, ptheCookie );
					}
				else
					{
					//Lookup the handler function fo this filename
					pHandler=handler_map[filename];
					//Call the default handler if we didn't get the filename
					if( !pHandler )
						{
						DebugMsg( "HTTPServer-embed","%d Call default handler\r\n",__LINE__);
						
						return pDefaultHandler( *this, ptheCookie );
						}
					//Found a handler
					else
						{
						DebugMsg( "HTTPServer-embed","%d Call user handler\r\n",__LINE__);
						return pHandler( *this, ptheCookie );
						}
					}
				}
	DebugMsg( "HTTPServer-embed","Error parsing request\r\n");
	return EHTTP_ERR_GENERIC;
	}


void ehttp::setSendFunc( size_t (*pS)(void *fd, const void *buf, size_t len, void *cookie) )
	{
	if( pS )
		pSend=pS;
	else
		pSend=ehttpSend;
	}

void ehttp::setRecvFunc( size_t (*pR)(void *fd, void *buf, size_t len, void *cookie) )
	{
	if( pR )
		pRecv=pR;
	else
		pRecv=ehttpRecv;
	}




map <string, string> & ehttp::getResponseHeader( void )
	{
	return response_header;
	}








