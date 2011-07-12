#pragma once
#include "../Generic/tools.h"
//#include "../Base/MemoryBuffer.h"
#include "../ContentList/ContentItemNew.h"

struct iconData {
	BYTE* thumbnailData;
	long thumbnailSize;
};

typedef enum _CONTENT_DVD_TYPE {
	CONTENT_DVD_TYPE_UNKNOWN	= 0,
	CONTENT_DVD_TYPE_XEX		= 1,
	CONTENT_DVD_TYPE_XBE		= 2,
	CONTENT_DVD_TYPE_DVD		= 3,
	CONTENT_DVD_TYPE_DATA		= 4
} CONTENT_DVD_TYPE;

class DVDItem;

class iDVDItemRequester{
public :
	virtual void ReadingDisc(){};
	virtual void ExtractCompleted(DVDItem*){};
	virtual void UpdateFileCount(DVDItem*){};
};

class DVDItem {
private :

	// Some basic info about this item
	iDVDItemRequester* requester;

	// DVD Info
	string Title;
	string fullTitle;
	bool haveSlot;
	bool haveThumb;
	iconData thumbData;
	iconData slotData;
	CONTENT_DVD_TYPE fileType;
	bool scan;
	bool scanComplete;
	bool includeUpdate;
	string currentFile;
	string destPath;


public :

	friend class DVDWorker;

	// Some properties
	void          setRequester(iDVDItemRequester* req)			{ requester = req;						 }
	void		  SetThumbData(iconData iData)					{ thumbData = iData;					 }
	void		  setTitle(wstring szTitle)						{ Title = wstrtostr(szTitle);			 }
	void		  setFullTitle(wstring szTitle)					{ fullTitle = wstrtostr(szTitle);		 }
	void		  setSlotData(iconData iData)					{ slotData = iData;						 }
	void		  setHaveSlot(bool slotLoaded)					{ haveSlot = slotLoaded;				 }
	void		  setHaveThumb(bool thumbLoaded)				{ haveThumb = thumbLoaded;				 }
	void		  setScan(bool bScan, bool iUpdate)				{ scan = bScan; includeUpdate = iUpdate; }
	void		  setScanComplete(bool cScan)					{ scanComplete=cScan;					 }
	void		  setCurrentFile(string file)					{ currentFile = file;					 }
	void		  setDestPath(string dest)						{ destPath = dest;						 }
	
	// Our constructors
	DVDItem(CONTENT_DVD_TYPE fType);

	// Methods
	string				getTitle()				{return Title;			}
	string				getFullTitle()			{return fullTitle;		}
	iconData			getSlotData()			{return slotData;		}
	iconData			getThumbData()			{return thumbData;		}
	string				getCurrentFile()		{return currentFile;	}
	bool				getIncludeUpdate()		{return includeUpdate;	}
	bool				getScanComplete()		{return scanComplete;	}
	string				getDestPath()			{return destPath;		}
	bool				getHasSlot()			{return haveSlot;		}
	bool				getHasThumb()			{return haveThumb;		}
	CONTENT_DVD_TYPE	getFileType()			{return fileType;		}


};