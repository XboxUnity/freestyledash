#include "stdafx.h"
#include "../Generic/xboxtools.h"
#include "../Generic/tools.h"
#include "../Debug/Debug.h"

#include "XFSD.h"

XFSD::XFSD()
{
	fHandle = NULL;
}

XFSD::~XFSD()
{
	if(fHandle != NULL)
		CloseXFSD();
}

HRESULT XFSD::OpenXFSD(string szFileLocation, bool fromMemory)
{
	// Open our XFSD file 
	if(!fromMemory){
		fopen_s(&fHandle, szFileLocation.c_str(), "rb");
		fread(&Header, sizeof(XFSDHeader), 1, fHandle);
	}else{
		BOOL retVal = XGetModuleSection( GetModuleHandle(NULL), szFileLocation.c_str(), &xfsdData, &xfsdSize);
		if(retVal == FALSE)
			return 1;
		memcpy(&Header, xfsdData, sizeof(XFSDHeader));
	}

	// Check our magic
	if(Header.Magic != XFSD_MAGIC){
		if(!fromMemory && fHandle != NULL)
			fclose(fHandle);

		return 2;
	}

	m_bFromMemory = fromMemory;

	// Read our entry table	
	EntryTable = (XFSDEntry*)malloc(sizeof(XFSDEntry) * Header.EntryCount);
	if(!fromMemory)
		fread(EntryTable, sizeof(XFSDEntry), Header.EntryCount, fHandle);
	else
		memcpy(EntryTable, (BYTE*)xfsdData + sizeof(XFSDHeader), sizeof(XFSDEntry) * Header.EntryCount);

	return S_OK;
}

HRESULT XFSD::GetMemberCount(int * MemberCount)
{
	// Return the member count
	*MemberCount = Header.EntryCount;
	return S_OK;
}

HRESULT XFSD::GetMemberMetadata(int MemberID, PXAVATAR_METADATA pMetadata)
{
	// Make sure the requested ID is within the appropriate range
	if(MemberID < 0 || MemberID > Header.EntryCount - 1)
		return S_FALSE;

	// Obtain the referenced Entry and extract file Offset
	XFSDEntry * CurrentEntry = &EntryTable[MemberID];
	DWORD dwOffset = CurrentEntry->Offset;

	// Goto offset and read data into pointer
	if(!m_bFromMemory){
		fseek(fHandle, dwOffset, SEEK_SET);
		fread(pMetadata, XFSD_METADATA_MAX, 1, fHandle);	
	}else{
		memcpy(pMetadata, (BYTE*)xfsdData + dwOffset, XFSD_METADATA_MAX);
	}

	return S_OK;
}

string XFSD::GetMemberName(int MemberID)
{
	// Make sure the requested ID is within the appropriate range
	if(MemberID < 0 || MemberID > Header.EntryCount - 1)
		return "";

	// Obtain the referenced XFSDEntry
	XFSDEntry * CurrentEntry = &EntryTable[MemberID];
	
	// Return the Member's name
	return CurrentEntry->MemberName;
}

HRESULT XFSD::CloseXFSD( void )
{
	// Release file handle and memory
	if(fHandle != NULL)
		fclose(fHandle);

	return S_OK;
}