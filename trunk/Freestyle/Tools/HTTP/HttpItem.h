#pragma once
#include "../Generic/tools.h"
#include "./Base/MemoryBuffer.h"

typedef enum _HTTP_PRIORITY {
	
	HTTP_PRIORITY_START   = 0,
	
	// These are our prioritys
	HTTP_PRIORITY_HIGH			= 0,
	HTTP_PRIORITY_ABOVENORMAL	= 1,
	HTTP_PRIORITY_NORMAL		= 2,
	HTTP_PRIORITY_BELOWNORMAL	= 3,
	HTTP_PRIORITY_LOW			= 4,
	
	HTTP_PRIOIRTY_END	  = 4,

	// Special case to trigger a instant download
	HTTP_PRIORITY_INSTANT = 5,
} HTTP_PRIORITY;

typedef enum _REQUEST_METHOD {
	REQUEST_METHOD_OPTIONS = 0,
	REQUEST_METHOD_GET     = 1,
	REQUEST_METHOD_HEAD    = 2,
	REQUEST_METHOD_POST    = 3,
	REQUEST_METHOD_PUT     = 4,
	REQUEST_METHOD_DELETE  = 5,
	REQUEST_METHOD_TRACE   = 6,
	REQUEST_METHOD_CONNECT = 7
} REQUEST_METHOD;

typedef enum _HTTP_TYPE {
	HTTP_TYPE_UNDEFINED,
	HTTP_TYPE_CONTENT
} HTTP_TYPE;

class HttpItem;

class iHttpItemRequester{
public :
	virtual void DownloadCompleted(HttpItem*){};
};

class HttpItem {
private :

	// Some basic info about this item
	iHttpItemRequester* requester;
	iHttpItemRequester* downloader;
	HTTP_PRIORITY       priority;
	REQUEST_METHOD      requestMethod;
	string              tag;
	DWORD               tag2;	
	bool				retainQueue;
	DWORD				queueID;
	HTTP_TYPE			typeID;
	ULONGLONG			timestamp;
	DWORD				expiration;
	string				contentHash;

	// Our header
	MemoryBuffer        headerData;

	// Http Info
	string              url;
	DWORD               responseCode;
	BOOL                completed;

	// This is our output from web
	BOOL         outputToMemory;
	MemoryBuffer outputMemoryData;
	string       outputPath;
	BOOL         outputOpen;
	HANDLE       outputFHandle;
	DWORD        outputBytesDownloaded;
	DWORD        outputBytesToDownload;

	void OpenOutputData();
	void CloseOutputData();
	void WriteOutputData(BYTE* Data, DWORD Size);

	// This is for our input if we want to upload
	BOOL         uploadFile;
	string		 inputName;
	BOOL         inputFromMemory;
	BYTE*        inputData;
	DWORD        inputDataLength;
	string       inputPath;
	BOOL         inputOpen;
	HANDLE       inputFHandle;	
	DWORD        inputBytesSent;
	DWORD        inputBytesToSend;

	void  OpenInputData();
	void  CloseInputData();
	DWORD ReadInputData(BYTE* Data, DWORD Size);
	DWORD GetInputLength();

	// For "POST"ing data
	map<string, string> postVars;

	CHAR*  URLEncode(const CHAR* str);
	CHAR*  URLDecode(const CHAR* str);
	string GetPostVarString();

public :

	friend class HTTPWorker;
	friend class HttpClient;
	friend class HTTPDownloader;

	// Some properties
	HTTP_PRIORITY GetPriority()                           { return priority;         }
	void          SetPriority(HTTP_PRIORITY Value)        { priority = Value;        }
	void          SetRequestMethod(REQUEST_METHOD Method) { requestMethod = Method;  }
	MemoryBuffer& GetMemoryBuffer()                       { return outputMemoryData; }
	DWORD         getResponseCode()                       { return responseCode;     }
	void          setRequester(iHttpItemRequester* req)   { requester = req;         }
	void		  setDownloader(iHttpItemRequester* req)  { downloader = req;		 }
	string&       getTag()                                { return tag;              }
	void          setTag(const char* Value)               { tag = Value;             }
	DWORD         getTag2()                               { return tag2;             }
	void          setTag2(DWORD Value)                    { tag2 = Value;            }
	string&       getSaveAsPath()                         { return outputPath;       }
	string&       getFullUrl()                            { return url;              }
	BOOL		  getOutputToMemory()					  { return outputToMemory;	 }
	
	string        getPath();
	string        getServer();

	// Retain Queue
	bool		  getRetainQueue()						  { return retainQueue;		 }
	void	      setRetainQueue(bool value)			  { retainQueue = value;	 }
	DWORD		  getQueueID()							  { return queueID;			 }
	void		  setQueueID( DWORD value )				  { queueID = value;		 }
	HTTP_TYPE	  getTypeID()							  { return typeID;			 }
	void		  setTypeID( HTTP_TYPE value )			  { typeID = value;			 }

	void		  setTimeStamp( ULONGLONG ft )			  { timestamp = ft;			 }
	ULONGLONG	  getTimeStamp()						  { return timestamp;		 }

	void		  setExpiration( DWORD value )			  { expiration = value;		 }
	DWORD		  getExpiration()						  { return expiration;		 }

	string&		  getContentHash()						  { return contentHash;		 }
	void		  setContentHash(string hash )			  { contentHash = hash;		 }

	bool HasObjectExpired();

	// Our constructors
	HttpItem(const CHAR* Url, const CHAR* OutputFilePath = NULL, 
		REQUEST_METHOD RequestMethod = REQUEST_METHOD_GET);

	// Methods
	void AddPostVar(const CHAR* Key, const CHAR* Value, ...);	
	void SetInputFile(const CHAR* FilePath, const CHAR* FileName);
	void SetInputFile(void* Buffer, DWORD Length, const CHAR* FileName);
	void SetOutputFile(const CHAR* FilePath = NULL);

	string CreateMultiPartPostVars(string& Boundary);
	string CreateMultiPartFileInfo(string& Boundary);
	string CreateMultiPartFooter(string& Boundary);
};