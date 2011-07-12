#pragma once
#include "stdafx.h"
#include "ContentMerger.h"
#include "../Generic/xboxtools.h"
#include "../Generic/tools.h"

ContentMerger::ContentMerger( void )
{


}

HRESULT ContentMerger::BackupGameData( string szBaseFolder )
{


	return S_OK;
}

HRESULT ContentMerger::ScanForContent( string szBaseFolder )
{
	// Start searching for foler names
	szBaseFolder = szBaseFolder + "\\";
	szBaseFolder = str_replaceallA(szBaseFolder, "\\\\", "\\");

	string szFind = szBaseFolder + "*";
    WIN32_FIND_DATA wfd; HANDLE hFind;
    hFind = FindFirstFile( szFind.c_str(), &wfd );
    if(hFind == INVALID_HANDLE_VALUE) return S_OK;

	// Loop and find each
	do {
		// Lets check if this is a directory 
		if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			
			CONTENT_SCANRESULTS * scanItem = new CONTENT_SCANRESULTS;;
			string szDiscoveredDir = szBaseFolder + wfd.cFileName + "\\";
			scanItem->szContentPath = szDiscoveredDir;
			scanItem->dwTitleId = strtoul(wfd.cFileName, NULL, 16);

			m_vScanResults.push_back(scanItem);
		}
	}while(FindNextFile(hFind, &wfd));

	// Done finding
	FindClose( hFind );

	// Now that we have our paths, and our title ids, let's process each one
	unsigned int nSize = m_vScanResults.size();
	for(unsigned int i = 0; i < nSize; i++)
	{
		string path = m_vScanResults.at(i)->szContentPath;
		DWORD titleId = m_vScanResults.at(i)->dwTitleId;
	}

	return S_OK;
}


