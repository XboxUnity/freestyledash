#include "stdafx.h"
#include "FileCallBackItem.h"

FileCallBackItem::FileCallBackItem() {

	// Setup some basic info
	requester = NULL;
	
	// FileCallBack Info
	FilesDone				= 0;
	TotalFiles				= 0;
	SizeDone				= 0;
	SizeTotal				= 0;
	OverallPercent			= 0;
	
	Current					= "";
	CurrentSize				= 0;
	CurrentSizeComplete		= 0;
	CurrentPercent			= 0;

	isDone					= false;
	isPrepDone				= false;
	
}
