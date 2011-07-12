#include "stdafx.h"
#include "ContentExtractor.h"
#include "ContentManager.h"
#include "../SQLite/FSDSql.h"
#include "../XEX/SPA.h"
#include "../XEX/Xbox360Container.h"

ContentExtractor::ContentExtractor(void) {

	// Initalize some variables
	paused = FALSE;
	InitializeCriticalSection(&lock);

	// Now create our worker thread
	CreateThread(CPU3_THREAD_2);
}

ContentExtractor::~ContentExtractor(void) {
	DeleteCriticalSection(&lock);
}

void ContentExtractor::AddContentItem(ContentItemNew* Item) {

	// Add our item
	EnterCriticalSection(&lock);
	itemQueue.push_back(Item);
	LeaveCriticalSection(&lock);
}
unsigned long ContentExtractor::Process(VOID* Param) {
	
	// Set some thread stuff
	SetThreadName("ContentExtractor");
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);

	// Loop forever!
	for(;;) {

		// Check if were paused
		if(paused) {
			Sleep(500);
			continue;
		}

		// Get our item from the queue
		if(itemQueue.size() == 0) {
			Sleep(500);
			continue;
		}

		EnterCriticalSection(&lock);
		ContentItemNew* item = itemQueue.at(0);
		itemQueue.erase(itemQueue.begin());
		LeaveCriticalSection(&lock);

		// Process our item
		ProcessItem(item);
		Sleep(500);
	}
}
void ContentExtractor::ProcessItem(ContentItemNew* Item) {
	
	// First make sure this is a xex
	if(Item->GetFileType(true) != CONTENT_FILE_TYPE_XEX_PENDING )
		return;

	bool bNxeArtIconFailed = false;

	DebugMsg("ContentExtractor", "Extracting .xex file");
	// Try and get slot stuff first if we can
	string nxeArtPath = Item->getRoot() + Item->getDirectory() + "nxeart";
	if(FileExists(nxeArtPath)) {
		DebugMsg("ContentExtractor", "Extracting from nxeart");
		// Read our Container info
		Xbox360Container container;
		if(container.OpenContainer(nxeArtPath) != S_OK) {
			goto LoadFromXEX;
		}
		DebugMsg("ContentExtractor", "nxeart opened.");
		// Make sure its the correct content type
		if(container.pContainerMetaData->ContentType != 
			XCONTENTTYPE_THEMATICSKIN) {

			container.CloseContainer();
			goto LoadFromXEX;
		}
		DebugMsg("ContentExtractor", "nxeart correct file.");
		// Check if theres a title name we can load
		if(wcslen(container.pContainerMetaData->TitleName) > 0) {
			DebugMsg("ContentExtractor", "Title Set");
			Item->setTitle(container.pContainerMetaData->TitleName);
		}
	
		// Check if theres a thumbnail we can load
		if(container.pContainerMetaData->ThumbnailSize > 0 && SETTINGS::getInstance().getDownloadIcon() == 1) {
			DebugMsg("ContentExtractor", "Icon Set");
			BYTE* thumbnailData = NULL; long thumbnailSize = 0;
			#ifdef _USING_DXT5
				ConvertImageInMemoryToDXT5Buffer(
					container.pContainerMetaData->Thumbnail, 
					container.pContainerMetaData->ThumbnailSize, 
					&thumbnailData, &thumbnailSize);
			#else if
				ConvertImageInMemoryToDXT1Buffer(
					container.pContainerMetaData->Thumbnail, 
					container.pContainerMetaData->ThumbnailSize, 
					&thumbnailData, &thumbnailSize);
			#endif

			FSDSql::getInstance().AddContentAsset(Item->GetItemId(), 
				ASSET_TYPE_ICON, ASSET_FILE_TYPE_DDS, 
				thumbnailData, thumbnailSize);

			Item->SetIconData(thumbnailData, thumbnailSize);
			Item->SetAssetFlag(CONTENT_ASSET_THUMBNAIL);
		} else if( container.pContainerMetaData->ThumbnailSize == 0 ) {
			// The thumbnail failed, so we're going to want to run this through the xex extraction process too
			bNxeArtIconFailed = true;
		}

		// Close our container
		container.CloseContainer();

		// Try and mount it to get our bg image and slot image
		if(container.Mount("NXEART") != S_OK) {
			if( bNxeArtIconFailed == true ) {
				goto LoadFromXEX;
			} else {
				Item->setFileType(CONTENT_FILE_TYPE_XEX);
				return;
			}
		}

		// Only if it doesnt already exists
		if(!(Item->GetAssetFlags() & CONTENT_ASSET_BACKGROUND) && SETTINGS::getInstance().getDownloadBackground() == 1) {
			DebugMsg("ContentExtractor", "Extracting background from nxeart");
			string nxebgPath = "NXEART:\\nxebg.jpg";
			if(FileExists(nxebgPath)){
				Item->SetAssetFlag(CONTENT_ASSET_BACKGROUND);
				string savePath = Item->getAssetDirectory() + "\\bg.dds";
				#ifdef _USING_DXT5
					ConvertImageFileToDXT5(savePath, nxebgPath);
				#else if
					ConvertImageFileToDXT1(nxebgPath, savePath);
				#endif
				FSDSql::getInstance().AddContentAsset(Item->GetItemId(), 
					ASSET_TYPE_BACKGROUND, ASSET_FILE_TYPE_DDS, 
					(VOID*)savePath.c_str());
				Item->SetBackgroundPath(savePath.c_str());
			}
		}

		string nxeslotPath = "NXEART:\\nxeslot.jpg";
		if(FileExists(nxeslotPath)){
			DebugMsg("ContentExtractor", "Extracting slot image from nxeart");
			string savePath = Item->getAssetDirectory() + "\\slot.dds";
			#ifdef _USING_DXT5
				ConvertImageFileToDXT5(savePath, nxeslotPath);
			#else if
				ConvertImageFileToDXT1(nxeslotPath, savePath);
			#endif
			FSDSql::getInstance().AddContentAsset(Item->GetItemId(), 
				ASSET_TYPE_NXE_SLOT, ASSET_FILE_TYPE_DDS, 
				(VOID*)savePath.c_str());
			Item->SetSlotPath(savePath.c_str());
			Item->SetAssetFlag(ITEM_UPDATE_TYPE_NXE_SLOT);
		}

		string nxebannerPath = "NXEART:\\nxebanner.jpg";
		if(SETTINGS::getInstance().getDownloadBanner() == 1)
		{
			if(FileExists(nxebannerPath)){
				DebugMsg("ContentExtractor", "Extracting Banner image from nxeart");
				string savePath = Item->getAssetDirectory() + "\\banner.dds";
				#ifdef _USING_DXT5
					ConvertImageFileToDXT5(savePath, nxebannerPath);
				#else if
					ConvertImageFileToDXT1(savePath, nxebannerPath);
				#endif
				FSDSql::getInstance().AddContentAsset(Item->GetItemId(), 
					ASSET_TYPE_BANNER, ASSET_FILE_TYPE_DDS, 
					(VOID*)savePath.c_str());
				Item->SetSlotPath(savePath.c_str());
				Item->SetAssetFlag(ITEM_UPDATE_TYPE_BANNER);
			}
		}

		string nxeboxartPath = "NXEART:\\nxeboxart.jpg";
		if(SETTINGS::getInstance().getDownloadBoxart() == 1)
		{
			if(FileExists(nxeboxartPath)){
				DebugMsg("ContentExtractor", "Extracting Boxart image from nxeart");
				string savePath = Item->getAssetDirectory() + "\\box.dds";
				#ifdef _USING_DXT5
					ConvertImageFileToDXT5(savePath, nxeboxartPath);
				#else if
					ConvertImageFileToDXT1(savePath, nxeboxartPath);
				#endif
				FSDSql::getInstance().AddContentAsset(Item->GetItemId(), 
					ASSET_TYPE_BOXCOVER, ASSET_FILE_TYPE_DDS, 
					(VOID*)savePath.c_str());
				Item->SetSlotPath(savePath.c_str());
				Item->SetAssetFlag(ITEM_UPDATE_TYPE_BOXART);
			}
		}

		// Unmount now
		container.UnMount();

		if(bNxeArtIconFailed == false) {
			// Our assets have successfully extracted, let's remove the pending status
			Item->setFileType(CONTENT_FILE_TYPE_XEX);

			// All done
			return;
		}
	}

LoadFromXEX:
	DebugMsg("ContentExtractor", "Extracting from xex");
	// Open our xex
	XeXtractor xex;
	string itemPath = Item->getRoot() + Item->getPath();
	if(xex.OpenXex(itemPath) != S_OK) return;
	DebugMsg("ContentExtractor", "xex opened");
	// Extract our spa
	BYTE* spaData = NULL; DWORD spaDataLength = 0;
	if(xex.ExtractTitleSPA(&spaData, &spaDataLength) != S_OK) {
		xex.CloseXex();
		Item->setFileType(CONTENT_FILE_TYPE_XEX);
		return;
	}
	DebugMsg("ContentExtractor", "spa extracted.");

	// We have our spa so lets close
	xex.CloseXex();

	// Read our spa
	SPA spa;
	if(spa.OpenSPA(spaData, spaDataLength) != S_OK) {
		spa.CloseSPA();
		free(spaData);
		Item->setFileType(CONTENT_FILE_TYPE_XEX);
		return;
	}
	DebugMsg("ContentExtractor", "Spa opened");
	// Get our title name
	string titleNameSpa = "";
	if(spa.GetTitleName(&titleNameSpa, LOCALE_ENGLISH) != S_OK) {
		spa.CloseSPA();
		free(spaData);
		Item->setFileType(CONTENT_FILE_TYPE_XEX);
		return;
	}
	

	// We have our title
	WCHAR* multiByte = strtowchar(titleNameSpa);
	Item->setTitle(multiByte);
	delete multiByte;
	DebugMsg("ContentExtractor", "Title Set");
	// Load our image

	BYTE* spaImage = NULL;	DWORD spaImageSize = 0;
	if(spa.GetTitleImage(&spaImage, &spaImageSize) != S_OK) {
		spa.CloseSPA();
		free(spaData);
		Item->setFileType(CONTENT_FILE_TYPE_XEX);
		return;
	}

	if(spaImageSize > 0 && SETTINGS::getInstance().getDownloadIcon() == 1)
	{
		// Convert ot dxt5
		if(spaImageSize > 0 && spaImage != NULL) {
			BYTE* thumbnailData = NULL; long thumbnailSize = 0;
			ConvertImageInMemoryToDXT5Buffer(spaImage, spaImageSize, 
				&thumbnailData, &thumbnailSize);

			FSDSql::getInstance().AddContentAsset(Item->GetItemId(), 
				ASSET_TYPE_ICON, ASSET_FILE_TYPE_DDS, 
				thumbnailData, thumbnailSize);

			Item->SetIconData(thumbnailData, thumbnailSize);
			Item->SetAssetFlag(CONTENT_ASSET_THUMBNAIL);
			
			// Can clean up the image now
			free(spaImage);
		}
		DebugMsg("ContentExtractor", "Thumbnail set");
	}
	
	// Clean up our mess
	spa.CloseSPA();
	free(spaData);

	Item->setFileType(CONTENT_FILE_TYPE_XEX);
}