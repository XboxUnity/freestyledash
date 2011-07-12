#pragma once
#include "../HTTP/HTTPDownloader.h"
#include "../HTTP/HttpItem.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"

#define VERSION_HEADER "XSERVER_ADVANCED_VERSION"
#define APPSPEC_HEADER "XSERVER_ADVANCED_APPSPEC"
#define APPSPEC_PREFIX "XEX_"
#define APPSPEC_PREFIX_CLASSIC "XBE_"
#define APPSPEC_URLTAG "KAIVECTOR"
#define VERSION_URLTAG "KAIVECTORVERSION"

class ContentKaiVector : public iHttpItemRequester
{
public:
	ContentKaiVector();
	~ContentKaiVector();

	static ContentKaiVector& getInstance()
	{
		static ContentKaiVector singleton;
		return singleton;
	}

	void Initialize( void );
	void Load(const string strPath);
	void Save(const string strPath);

	void DownloadAppSpec(string strUrl);
	void DownloadAppSpecVersion(string strUrl);

	void AddTitle(DWORD aTitleId, string& aVector);
	bool GetVectorByTitleId(DWORD aTitleId, string& aVector);
	bool GetTitleIdByVector(string& aVector, DWORD * aTitleId );
	bool ContainsTitle(DWORD aTitleId);
	bool IsEmpty();

	void DownloadCompleted(HttpItem * itm);

protected:

	string GetBasePath();
	string vectorDataPath;

	int m_nVersion;
	
	typedef std::map<DWORD, string> TITLEVECTORMAP;
	typedef std::map<string, DWORD> VECTORTITLEMAP;
	TITLEVECTORMAP m_mapTitles;
	VECTORTITLEMAP m_mapVectors;
	CRITICAL_SECTION m_critical;
	bool m_bDirty;

	ContentKaiVector(const ContentKaiVector&);						// Prevent copy-construction
	ContentKaiVector& operator=(const ContentKaiVector&);		    // Prevent assignment
};