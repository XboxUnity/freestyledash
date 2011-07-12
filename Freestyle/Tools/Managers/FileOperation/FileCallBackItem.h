#pragma once
#include "../../Generic/tools.h"


class FileCallBackItem;

class iFileCallBackItemRequester {
public :
	virtual void UpdateInfo(FileCallBackItem*){};
};

class FileCallBackItem {
private :

	// Some basic info about this item
	iFileCallBackItemRequester* requester;

	// Progress Info
	int FilesDone;
	int TotalFiles;
	ULONGLONG SizeDone;
	ULONGLONG SizeTotal;
	int OverallPercent;
	
	string Current;
	ULONGLONG CurrentSize;
	ULONGLONG CurrentSizeComplete;
	int CurrentPercent;

	bool isDone;
	bool isPrepDone;
	bool firstAfterPrep;


public :

	friend class FileCallBack;

	// Some properties
	void        setRequester(iFileCallBackItemRequester* req)		{ requester = req;				}
	void		setCurrent(string FileName)							{ Current = FileName;			}
	void		setCurrentSize(ULONGLONG size)						{ CurrentSize = size;			}
	void		setCurrentSizeComplete(ULONGLONG size)				{ CurrentSizeComplete = size;	}
	void		setCurrentPercent(int percent)						{ CurrentPercent = percent;		}
	void		setFilesDone(int done)								{ FilesDone = done;				}
	void		setTotalFiles(int Total)							{ TotalFiles = Total;			}
	void		setSizeDone(ULONGLONG size)							{ SizeDone = size;				}
	void		setSizeTotal(ULONGLONG size)						{ SizeTotal = size;				}
	void		setOverallPercent(int percent)						{ OverallPercent = percent;		}
	void		setIsDone(bool complete)							{ isDone = complete;			}
	void		setPrepDone(bool complete)							{ isPrepDone = complete;		}
	void		setFirstAfterPrep(bool complete)					{ firstAfterPrep = complete;	}

	
	// Our constructors
	FileCallBackItem();

	// Methods
	string		getCurrent()							{ return Current;				}
	ULONGLONG	getCurrentSize()						{ return CurrentSize;			}
	ULONGLONG	getCurrentSizeComplete()				{ return CurrentSizeComplete;	}
	int			getCurrentPercent()						{ return CurrentPercent;		}
	int			getFilesDone()							{ return FilesDone;				}
	int			getTotalFiles()							{ return TotalFiles;			}
	ULONGLONG	getSizeDone()							{ return SizeDone;				}
	ULONGLONG	getSizeTotal()							{ return SizeTotal;				}
	int			getOverallPercent()						{ return OverallPercent;		}
	bool		getIsDone()								{ return isDone;				}
	bool		getIsPrepDone()							{ return isPrepDone;			}
	bool		getFirstAfterPrep()						{ return firstAfterPrep;		}


};