#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "GPD.h"

HRESULT GPD::OpenGPD(string filePath){

	// Open our spa file 
	fopen_s(&fHandle, filePath.c_str(), "rb");
	fread(&Header, sizeof(XBDFHeader), 1, fHandle);

	// Check our magic
	if(Header.Magic != 0x58444246){
		fclose(fHandle);
		return 2;
	}

	// Read our entry table	
	EntryTable = (XBDFEntry*)malloc(sizeof(XBDFEntry) * Header.EntryMax);
	fread(EntryTable, sizeof(XBDFEntry), Header.EntryMax, fHandle);

	// Read our free table
	FreeTable = (XBDFFileLoc*)malloc(sizeof(XBDFFileLoc) * Header.FreeMax);
	fread(FreeTable, sizeof(XBDFFileLoc), Header.FreeMax, fHandle);

	// Clear currentTable
	currentTable = -1;

	// All done
	return 1;	
};
HRESULT GPD::CloseGPD(){
	fclose(fHandle);
	return 1;
};
HRESULT GPD::GetSection(short NameSpace, __int64 Id, byte** Data, int* Size){
	
	// Loop through and find our id
	XBDFEntry* entry = NULL; bool found = false;
	for(int x = 0; x < Header.EntryCurrent; x++){
		if(EntryTable[x].NameSpace == NameSpace &&
			EntryTable[x].Id == Id) {
			entry = &EntryTable[x]; 
			found = true; break;
		}
	}

	if(!found){
		DebugMsg("GPD", "Section Not Found!");	
		return 2;
	}

	*Data = (byte*)malloc(entry->Size);
	*Size = entry->Size;

	int headerSize = sizeof(XBDFHeader) + 
		(sizeof(XBDFEntry) * Header.EntryMax) + 
		(sizeof(XBDFFileLoc) * Header.FreeMax);
	fseek(fHandle, headerSize + entry->Offset, SEEK_SET);
	fread(*Data, entry->Size, 1, fHandle);

	return 1;
};