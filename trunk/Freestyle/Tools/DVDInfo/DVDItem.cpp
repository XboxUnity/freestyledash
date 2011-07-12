#include "stdafx.h"
#include "DVDItem.h"
#include "../ContentList/ContentItemNew.h"

DVDItem::DVDItem(CONTENT_DVD_TYPE fType) {

	// Setup some basic info
	requester = NULL;
	
	// Http Info
	Title					=	"";
	thumbData.thumbnailData	=	NULL;
	thumbData.thumbnailSize	=	NULL;
	slotData.thumbnailData	=	NULL;
	slotData.thumbnailSize	=	NULL;
	haveSlot				=	false;
	haveThumb				=	false;
	fileType				=	fType;
	scan					=	false;
	includeUpdate			=	false;
}
	
