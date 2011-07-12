#include "stdafx.h"

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "SPA.h"

void SPA::fRead(void* DstBuf, size_t ElementSize, size_t Count) {

	// If its a file lets do that
	if(isFile) {
		fread(DstBuf, ElementSize, Count, fHandle);
		return;
	}

	// Its not a file its memory
	memcpy(DstBuf, CurrentPosition, ElementSize * Count);
	CurrentPosition += (ElementSize * Count);
}
void SPA::fSeek(long Offset, int Origin) {

	// If its a file lets do that
	if(isFile) {
		fseek(fHandle, Offset, Origin);
		return;
	}

	if(Origin == SEEK_CUR)
		CurrentPosition += Offset;
	else if(Origin == SEEK_END)
		CurrentPosition = SPAData + (SPADataSize - Offset);
	else if(Origin == SEEK_SET)
		CurrentPosition = SPAData + Offset;
}
HRESULT SPA::OpenSPA(string filePath){

	// This is a file
	isFile = TRUE;
	SPAData = NULL;
	CurrentPosition = NULL;

	// Open our spa file
	fopen_s(&fHandle, filePath.c_str(), "rb");
	fRead(&Header, sizeof(XBDF_HEADER), 1);

	// Check our magic
	if(Header.Magic != 0x58444246){
		fclose(fHandle);
		return 1;
	}

	// Read our entry table	
	EntryTable = (XBDF_ENTRY*)malloc(sizeof(XBDF_ENTRY) * Header.EntryMax);
	fRead(EntryTable, sizeof(XBDF_ENTRY), Header.EntryMax);

	// Read our free table
	FreeTable = (XBDF_FILE_LOC*)malloc(sizeof(XBDF_FILE_LOC) * Header.FreeMax);
	fRead(FreeTable, sizeof(XBDF_FILE_LOC), Header.FreeMax);

	// Clear currentTable
	currentTable = -1;

	// All done
	return S_OK;	
}
HRESULT SPA::OpenSPA(BYTE* SPABuffer, DWORD SPASize){

	// We arent a file
	isFile = FALSE;
	SPAData = SPABuffer;
	SPADataSize = SPASize;
	CurrentPosition = SPAData;

	// Get our spa header
	fRead(&Header, sizeof(XBDF_HEADER), 1);
	if(Header.Magic != 0x58444246) return 1;

	// Read our entry table	
	EntryTable = (XBDF_ENTRY*)malloc(sizeof(XBDF_ENTRY) * Header.EntryMax);
	fRead(EntryTable, sizeof(XBDF_ENTRY), Header.EntryMax);

	// Read our free table
	FreeTable = (XBDF_FILE_LOC*)malloc(sizeof(XBDF_FILE_LOC) * Header.FreeMax);
	fRead(FreeTable, sizeof(XBDF_FILE_LOC), Header.FreeMax);

	// Clear currentTable
	currentTable = -1;

	// All done
	return S_OK;	
}
HRESULT SPA::CloseSPA(){

	// If its a file close it
	if(isFile)
		fclose(fHandle);

	// Clean up some stuff
	if(EntryTable!= NULL) {
		free(EntryTable);
		EntryTable = NULL;
	}
	if(FreeTable != NULL) { 
		free(FreeTable);
		FreeTable = NULL;
	}

	// Clear the non file stuff
	SPAData = NULL;
	SPADataSize = 0;
	CurrentPosition = NULL;	

	return S_OK;
}
HRESULT SPA::GetSection(WORD NameSpace, INT64 Id, BYTE** Data, DWORD* Size){
	
	// Loop through and find our id
	XBDF_ENTRY* entry = NULL; BOOL found = FALSE;
	for(DWORD x = 0; x < Header.EntryCurrent; x++){
		if(EntryTable[x].NameSpace == NameSpace &&
			EntryTable[x].Id == Id) {
			entry = &EntryTable[x]; 
			found = true; break;
		}
	}

	if(!found){
		DebugMsg("SPA", "Section Not Found!");	
		return 1;
	}

	*Data = (BYTE*)malloc(entry->Size);
	*Size = entry->Size;

	DWORD headerSize = sizeof(XBDF_HEADER) + 
		(sizeof(XBDF_ENTRY) * Header.EntryMax) + 
		(sizeof(XBDF_FILE_LOC) * Header.FreeMax);
	fSeek(headerSize + entry->Offset, SEEK_SET);
	fRead(*Data, entry->Size, 1);

	return S_OK;
}
HRESULT SPA::GetStringTable(INT64 Id) {
	
	BYTE* Data;	DWORD Size = 0;
	if(GetSection(SPA_SECTION_STRINGTABLES, Id, 
		&Data, &Size) != S_OK){
			DebugMsg("SPA", "String Tables Section not found! id: %x", Id);
			return 1;
	}

	DebugMsg("SPA", "Loaded string table - id: %x size: %d", Id, Size);		

	// Figure out how many strings we have
	m_Entries.clear();
	WORD nSize = ((SPA_XSTR_HEADER*)Data)->StringCount;
	long currentAddress = (long)Data + sizeof(SPA_XSTR_HEADER);

	WORD tableId = 0x0000;
	WORD tableValLen = 0x0000;

	for(DWORD x = 0; x < nSize; x++)
	{
		tableId = ((SPA_STRING_ENTRY*)currentAddress)->Id;
		tableValLen = ((SPA_STRING_ENTRY*)currentAddress)->StringLength;

		CHAR* tableValue = (CHAR*)malloc(tableValLen + 1);
		
		memset(tableValue, 0, tableValLen + 1);
		memcpy(tableValue, ((SPA_STRING_ENTRY*)currentAddress)->String, tableValLen);

		string strTemp = tableValue;
		m_Entries.insert(map<WORD, string>::value_type(tableId, strTemp));
		
		free(tableValue);
		currentAddress += (2 * sizeof(WORD) + tableValLen);
	}

	free(Data);
	DebugMsg("SPA", "Created String Map with '%d' elements.", m_Entries.size());

	return S_OK;
}
HRESULT SPA::GetTableValue(INT64 Id, string* value, DWORD locale) {
	
	if(currentTable == -1){

		// First we gotta get our XSTC
		BYTE* Data;	DWORD Size = 0;
		if(GetSection(SPA_SECTION_METADATA, SPA_MAGIC_XSTC, 
			&Data, &Size) != S_OK) {
				DebugMsg("SPA", "Could not load XSTC section");	
				return 1;	
		}

		// Get our language
		DWORD defaultLanguage;
		if(locale == LOCALE_DEFAULT)
			defaultLanguage = ((SPA_XSTC*)Data)->DefaultLanguage;
		else
			defaultLanguage = locale;

		currentTable = (short)defaultLanguage;

		if(GetStringTable(defaultLanguage) != S_OK) return 3;
	}

	if(m_Entries.find((short)Id) != m_Entries.end()){
		*value = m_Entries[(short)Id];
		return S_OK;
	}

	// Not Found
	return 2;
}
HRESULT SPA::GetTitleName(string* Title, DWORD locale) {
	return GetTableValue(0x8000, Title, locale);
}
HRESULT SPA::GetImage(INT64 Id, BYTE** ImageData, DWORD* Size) {

	if(GetSection(SPA_SECTION_IMAGES, Id, 
		ImageData, Size) != S_OK){
			DebugMsg("SPA", "Image Section not found! id: %x", Id);
			return 1;
	}

	return S_OK;
}
HRESULT SPA::GetTitleImage(BYTE** ImageData, DWORD* Size) {
	return GetImage(0x00008000, ImageData, Size);
}