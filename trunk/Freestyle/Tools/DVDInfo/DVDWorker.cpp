#include "stdafx.h"

#include "DVDItem.h"
#include "../Threads/CThread.h"
#include "../Debug/Debug.h"
#include "DVDWorker.h"
#include "../ContentList/ContentItemNew.h"
#include "../XEX/SPA.h"
#include "../XEX/Xbox360Container.h"
#include "../Managers/FileOperation/FileOperationManager.h"

DVDWorker::DVDWorker() {

	m_WaitingToStart = FALSE;
	m_CurrentItem = NULL;
	m_IsBusy = FALSE;
	m_IsDisposed = FALSE;
	CreateThread(CPU3_THREAD_2);
}
DVDWorker::~DVDWorker() {
	m_IsDisposed = TRUE;
}
void DVDWorker::StartItem(DVDItem* newItem) {

	// Wait till we are not busy
	while(IsBusy())
		Sleep(100);

	m_WaitingToStart = TRUE;
	m_CurrentItem = newItem;
}

void DVDWorker::SetNull()
{
	m_CurrentItem->setTitle(L"");
	m_CurrentItem->setHaveSlot(false);
	m_CurrentItem->setHaveThumb(false);
	if(m_CurrentItem->requester != NULL)
		m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
}

void DVDWorker::ExtractCurrentVideoDVDItem() {
	m_CurrentItem->setTitle(strtowstr("Video DVD"));
	// Trigger the download complete function
	if(m_CurrentItem->requester != NULL)
		m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);	
}

void DVDWorker::ExtractCurrentDataDVDItem() {
	m_CurrentItem->setTitle(strtowstr("Data DVD/CD"));
	// Trigger the download complete function
	if(m_CurrentItem->requester != NULL)
		m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);	
}

void DVDWorker::ExtractCurrentUnknownItem() {
	m_CurrentItem->setTitle(strtowstr("Unrecognized Disc"));
	// Trigger the download complete function
	if(m_CurrentItem->requester != NULL)
		m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);	
}

void DVDWorker::ExtractCurrentClassicItem() {
	string copydvdname;
	int namelength;
	m_CurrentItem->setTitle(L"Xbox Classic DVD");
	XboxExecutable xbe;
	xbe.OpenExecutable("dvd:\\default.xbe");
	xbe.GetTitleName(&copydvdname, &namelength);
	wstring Title = strtowstr(copydvdname);
	wstring fullTitle = sprintfaW(L"Play %s", strtowstr(copydvdname).c_str());
	m_CurrentItem->setTitle(Title);
	m_CurrentItem->setFullTitle(fullTitle);

	VOID* classicImage = NULL;	int classicImageSize = 0;
	xbe.GetGameThumbnail(&classicImage, &classicImageSize);


	// Convert ot dxt5
	if(classicImageSize > 0 && classicImage != NULL) {
		BYTE* thumbnailData = NULL; long thumbnailSize = 0;
		ConvertImageInMemoryToDXT5Buffer((BYTE*)classicImage, (long)classicImageSize, 
			&thumbnailData, &thumbnailSize);

		iconData dataIcon;
		dataIcon.thumbnailData = thumbnailData;
		dataIcon.thumbnailSize = thumbnailSize;
		m_CurrentItem->SetThumbData(dataIcon);
		m_CurrentItem->setHaveThumb(true);
	}
	// Trigger the download complete function
	if(m_CurrentItem->requester != NULL)
		m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);	
}
void DVDWorker::ScanDVDContents() {

	XamSetDvdSpindleSpeed(DVD_SPEED_12X);
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = "dvd:\\*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (strcmp(findFileData.cFileName, "$SystemUpdate") != 0)
				{
					string source = sprintfaA("dvd:\\%s", findFileData.cFileName);
					string dest = sprintfaA("%s\\%s", m_CurrentItem->getDestPath().c_str(), findFileData.cFileName);
					RecursiveMkdir(dest);
					m_CurrentItem->setCurrentFile(source);
					m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
					RecursiveScan(source, dest);					
				} else if (strcmp(findFileData.cFileName, "$SystemUpdate") == 0 && m_CurrentItem->getIncludeUpdate())
				{
					string source = sprintfaA("dvd:\\%s", findFileData.cFileName);
					string dest = sprintfaA("%s\\%s", m_CurrentItem->getDestPath().c_str(), findFileData.cFileName);
					RecursiveMkdir(dest);
					m_CurrentItem->setCurrentFile(source);
					m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
					RecursiveScan(source, dest);
				}
			} else {
				string source = sprintfaA("dvd:\\%s", findFileData.cFileName);
				string dest = sprintfaA("%s\\%s", m_CurrentItem->getDestPath().c_str(), findFileData.cFileName);
				m_CurrentItem->setCurrentFile(source);
				m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
				FileOperationManager::getInstance().AddFileOperation(source, dest, false);
			}
		} while (FindNextFile(hFind, &findFileData));
	}FindClose(hFind);
	m_CurrentItem->setScanComplete(true);
	m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
	FileOperationManager::getInstance().DoWork();
}

void DVDWorker::RecursiveScan(string Path, string destPath)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = Path + "\\*";
	searchcmd = str_replaceallA(searchcmd,"\\\\","\\");
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (strcmp(findFileData.cFileName, "$SystemUpdate") != 0)
				{
					string source = sprintfaA("%s\\%s",Path.c_str(), findFileData.cFileName);
					string dest = sprintfaA("%s\\%s",destPath.c_str(), findFileData.cFileName);
					RecursiveMkdir(dest);
					m_CurrentItem->setCurrentFile(source);
					m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
					RecursiveScan(source, dest);
				} else if (strcmp(findFileData.cFileName, "$SystemUpdate") == 0 && m_CurrentItem->getIncludeUpdate())
				{
					string source = sprintfaA("%s\\%s",Path.c_str(), findFileData.cFileName);
					string dest = sprintfaA("%s\\%s",destPath.c_str(), findFileData.cFileName);
					RecursiveMkdir(dest);
					m_CurrentItem->setCurrentFile(source);
					m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
					RecursiveScan(source, dest);
				}
			} else {
				string source = sprintfaA("%s\\%s",Path.c_str(), findFileData.cFileName);
				string dest = sprintfaA("%s\\%s", destPath.c_str(), findFileData.cFileName);
				m_CurrentItem->setCurrentFile(source);
				m_CurrentItem->requester->UpdateFileCount(m_CurrentItem);
				FileOperationManager::getInstance().AddFileOperation(source, dest, false);				
			}
		} while (FindNextFile(hFind, &findFileData));
	}FindClose(hFind);
}
void DVDWorker::ExtractCurrent360Item() {


	// Try and get slot stuff first if we can
	string nxeArtPath = "dvd:\\nxeart";
	m_CurrentItem->setTitle(L"Xbox 360 DVD");
	if(FileExists(nxeArtPath)) {
		
		// Read our Container info
		Xbox360Container container;
		if(container.OpenContainer(nxeArtPath) != S_OK) {
			goto LoadFromXEX;
		}

		// Make sure its the correct content type
		if(container.pContainerMetaData->ContentType != 
			XCONTENTTYPE_THEMATICSKIN) {

			container.CloseContainer();
			goto LoadFromXEX;
		}

		// Check if theres a title name we can load
		if(wcslen(container.pContainerMetaData->TitleName) > 0) {
			wstring Title = container.pContainerMetaData->TitleName;
			wstring fullTitle = sprintfaW(L"Play %s", container.pContainerMetaData->TitleName);
			m_CurrentItem->setFullTitle(fullTitle);
			m_CurrentItem->setTitle(Title);
		}

		// Check if theres a thumbnail we can load
		if(container.pContainerMetaData->ThumbnailSize > 0) {
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
			iconData dataIcon;
			dataIcon.thumbnailData = thumbnailData;
			dataIcon.thumbnailSize = thumbnailSize;
			m_CurrentItem->SetThumbData(dataIcon);
			m_CurrentItem->setHaveThumb(true);
		}

		// Close our container
		container.CloseContainer();

		// Try and mount it to get our bg image and slot image
		if(container.Mount("NXEART") != S_OK) {
			if(m_CurrentItem->requester != NULL)
				m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
			return;
		}

		string nxeslotPath = "NXEART:\\nxeslot.jpg";
		if(FileExists(nxeslotPath)){
			BYTE* thumbnailData = NULL; long thumbnailSize = 0;
			#ifdef _USING_DXT5
				ConvertImageFileToDXT5Buffer(nxeslotPath, &thumbnailData, &thumbnailSize);
			#else if
				ConvertImageFileToDXT1(nxeslotPath, &thumbnailData, &thumbnailSize);
			#endif
			iconData dataIcon;
			dataIcon.thumbnailData = thumbnailData;
			dataIcon.thumbnailSize = thumbnailSize;
			m_CurrentItem->setSlotData(dataIcon);
			m_CurrentItem->setHaveSlot(true);
		}

		// Unmount now
		container.UnMount();
		
		// Trigger the download complete function
		if(m_CurrentItem->requester != NULL)
			m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
		// All done
		return;
	}

LoadFromXEX:

	// Open our xex
	XeXtractor xex;
	if(xex.OpenXex("dvd:\\default.xex") != S_OK) {
		if(m_CurrentItem->requester != NULL)
			m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
		return;
	}

	// Extract our spa
	BYTE* spaData = NULL; DWORD spaDataLength = 0;
	if(xex.ExtractTitleSPA(&spaData, &spaDataLength) != S_OK) {
		xex.CloseXex();
		if(m_CurrentItem->requester != NULL)
			m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
		return;
	}

	// We have our spa so lets close
	xex.CloseXex();

	// Read our spa
	SPA spa;
	if(spa.OpenSPA(spaData, spaDataLength) != S_OK) {
		spa.CloseSPA();
		free(spaData);
		if(m_CurrentItem->requester != NULL)
			m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
		return;
	}

	// Get our title name
	string titleNameSpa = "";
	if(spa.GetTitleName(&titleNameSpa, LOCALE_ENGLISH) != S_OK) {
		spa.CloseSPA();
		free(spaData);
		if(m_CurrentItem->requester != NULL)
			m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
		return;
	}

	// We have our title
	WCHAR* multiByte = strtowchar(titleNameSpa);
	wstring fullTitle = sprintfaW(L"Play %s", multiByte);
	m_CurrentItem->setTitle(multiByte);
	m_CurrentItem->setFullTitle(fullTitle);
	delete multiByte;

	// Load our image
	BYTE* spaImage = NULL;	DWORD spaImageSize = 0;
	if(spa.GetTitleImage(&spaImage, &spaImageSize) != S_OK) {
		spa.CloseSPA();
		free(spaData);
		if(m_CurrentItem->requester != NULL)
			m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
		return;
	}

	// Convert ot dxt5
	if(spaImageSize > 0 && spaImage != NULL) {
		BYTE* thumbnailData = NULL; long thumbnailSize = 0;
		ConvertImageInMemoryToDXT5Buffer(spaImage, spaImageSize, 
			&thumbnailData, &thumbnailSize);

		iconData dataIcon;
		dataIcon.thumbnailData = thumbnailData;
		dataIcon.thumbnailSize = thumbnailSize;
		m_CurrentItem->SetThumbData(dataIcon);
		m_CurrentItem->setHaveThumb(true);
		
		// Can clean up the image now
		free(spaImage);
	}

	// Clean up our mess
	spa.CloseSPA();
	free(spaData);

	// Trigger the download complete function
	if(m_CurrentItem->requester != NULL)
		m_CurrentItem->requester->ExtractCompleted(m_CurrentItem);
}
unsigned long DVDWorker::Process(void* parameter) {

	DebugMsg("DVDWorker", "Process %d", CONTENT_DVD_TYPE_XBE);
	
	// Set our name
	SetThreadName("DVDWorker");

	// Now loop while we are not disposed
	while(!m_IsDisposed){

		// If our item is not null lets download
		if(m_CurrentItem != NULL)
		{
			m_IsBusy = TRUE;
			m_WaitingToStart = FALSE;

			if (m_CurrentItem->scan)
			{
				ScanDVDContents();
			} else {
				// Extract
				if (m_CurrentItem->getFileType() == CONTENT_DVD_TYPE_XEX)
				{
					if(m_CurrentItem->requester != NULL)
						m_CurrentItem->requester->ReadingDisc();
					ExtractCurrent360Item();
				} else if (m_CurrentItem->getFileType() == CONTENT_DVD_TYPE_XBE)
				{
					DebugMsg("DVDWorker", "XBE Extract");
					if(m_CurrentItem->requester != NULL)
						m_CurrentItem->requester->ReadingDisc();
					ExtractCurrentClassicItem();
				} else if (m_CurrentItem->getFileType() == CONTENT_DVD_TYPE_DVD) 
				{
					DebugMsg("DVDWorker", "DVD Extract");
					if(m_CurrentItem->requester != NULL)
						m_CurrentItem->requester->ReadingDisc();
					ExtractCurrentVideoDVDItem();
				} else if (m_CurrentItem->getFileType() == CONTENT_DVD_TYPE_DATA)
				{
					DebugMsg("DVDWorker", "Data DVD Extract");
					if(m_CurrentItem->requester != NULL)
						m_CurrentItem->requester->ReadingDisc();
					ExtractCurrentDataDVDItem();
				} else 
				{
					DebugMsg("DVDWorker", "Unrecognized Disc");
					if(m_CurrentItem->requester != NULL)
						m_CurrentItem->requester->ReadingDisc();
					ExtractCurrentUnknownItem();
				}
			}

			// Delete our item
			//if(m_CurrentItem != NULL)
			//	delete m_CurrentItem;

			// Set our item as null
			m_CurrentItem = NULL;

			// Waiting to start
			m_WaitingToStart = FALSE;
			m_IsBusy = FALSE;
		}

		// Sleep
		Sleep(100);
	}

	// Out of our loop
	return 0;
}