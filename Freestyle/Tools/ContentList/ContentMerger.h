#include "../Generic/xboxtools.h"
#include "../Generic/tools.h"


// Define paths to be used by ContentMerger
#define CONTENT_MERGER_DATAPATH "game:\\Data\\UserContent\\"
#define CONTENT_MERGER_SCANPATH "game:\\Data\\UserContent\\ScanData\\"
#define CONTENT_MERGER_BACKUPPATH "game:\\Data\\UserContent\\Backup"

// Filename definitions for artwork and videos
#define CONTENT_MERGER_ICON "icon"
#define CONTENT_MERGER_BOXART "box"
#define CONTENT_MERGER_BANNER "banner"
#define CONTENT_MERGER_BKG "bg"
#define CONTENT_MERGER_SLOT "slot"
#define CONTENT_MERGER_SCREENSHOT "screenshot"
#define CONTENT_MERGER_PREVIEW "preview"

// Filename for Game Info XML
#define CONTENT_MERGER_GAMEINFO "gameinfo.xml"

typedef struct _CONTENT_SCANRESULTS
{
	string szContentPath;
	DWORD dwTitleId;
} CONTENT_SCANRESULTS;

class ContentMerger 
{
public:
	ContentMerger();
	~ContentMerger() {}
	HRESULT BackupGameData( string szBasePath );
	HRESULT ScanForContent( string szBasePath );
private:
	
	vector<CONTENT_SCANRESULTS *> m_vScanResults;
	//vector<string> m_vScanResults;

protected:

};