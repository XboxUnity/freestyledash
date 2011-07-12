#include "stdafx.h"

#include "PkZip.h"

uint SwapUint(uint Value){
	return (uint)(((Value & 0xFF000000) >> 24) | ((Value & 0xFF0000) >> 8) | 
		((Value & 0xFF00) << 8) | ((Value & 0xFF) << 24));
};

ushort SwapUShort(ushort Value){
	return (ushort)(((Value & 0xFF00) >> 8) | ((Value & 0xFF) << 8));
};

HRESULT PkZip::Open(string ZipPath) {

	// Check if the file exists first
	if(!FileExists(ZipPath)) {
		DebugMsg("PkZip::Open", "File does not exist");
		return 1;
	}

	// Open our file up
	if(fopen_s(&fHandle, ZipPath.c_str(), "rb") != 0){
		DebugMsg("PkZip::Open", "Failed to open file");
		return 1;
	}

	// Goto the end locator
	fseek(fHandle, 0 - sizeof(EndLocator), SEEK_END);
	fread(&endLocator, sizeof(EndLocator), 1, fHandle);
	if(endLocator.Signature != EndLocatorSig) {
		fclose(fHandle);
		DebugMsg("PkZip::Open", "Invalid EndLocator Signature");
		return 2;
	}
	SwapEndLocator(&endLocator);

	// Allocate our space for the dir entries
	dirEntries = new DirEntry[endLocator.EntriesInDirectory];
	fseek(fHandle, endLocator.DirectoryOffset, SEEK_SET);
	for(int x = 0; x < endLocator.EntriesInDirectory; x++){
		// Read our entry
		fread(&dirEntries[x], sizeof(DirEntry) - sizeof(char*), 1, fHandle);
		if(dirEntries[x].Signature != DirEntrySig){
			fclose(fHandle);
			DebugMsg("PkZip::Open", "Invalid DirEntry Signature");
			return 3;
		}
		SwapDirEntry(&dirEntries[x]);

		// Read file name from entry
		dirEntries[x].FileName = (char*)malloc(dirEntries[x].FileNameLength + 1);
		fread(dirEntries[x].FileName, sizeof(char), 
			dirEntries[x].FileNameLength, fHandle);
		dirEntries[x].FileName[dirEntries[x].FileNameLength] = 0x00;
	}

	// All done
	return S_OK;
};

void PkZip::Close() {
	fclose(fHandle);
};

void PkZip::SwapEndLocator(EndLocator* el){
	el->DiskNumber = SwapUShort(el->DiskNumber);
	el->StartDiskNumber = SwapUShort(el->StartDiskNumber);
	el->EntriesOnDisk = SwapUShort(el->EntriesOnDisk);
	el->EntriesInDirectory = SwapUShort(el->EntriesInDirectory);
	el->DirectorySize = SwapUint(el->DirectorySize);
	el->DirectoryOffset = SwapUint(el->DirectoryOffset);
	el->CommentLength = SwapUShort(el->CommentLength);
};
void PkZip::SwapFileEntry(FileEntry* fe){
	fe->Version = SwapUShort(fe->Version);
	fe->Flags = SwapUShort(fe->Flags);
	fe->Compression = SwapUShort(fe->Compression);
	fe->FileTime = SwapUShort(fe->FileTime);
	fe->FileDate = SwapUShort(fe->FileDate);
	fe->Crc = SwapUint(fe->Crc);
	fe->CompressedSize = SwapUint(fe->CompressedSize);
	fe->UncompressedSize = SwapUint(fe->UncompressedSize);
	fe->FileNameLength = SwapUShort(fe->FileNameLength);
	fe->ExtraFieldLength = SwapUShort(fe->ExtraFieldLength);
};
void PkZip::SwapDirEntry(DirEntry* de){
	de->VersionMadeBy = SwapUShort(de->VersionMadeBy);
	de->VersionToExtract = SwapUShort(de->VersionToExtract);
	de->Flags = SwapUShort(de->Flags);
	de->Compression = SwapUShort(de->Compression);
	de->FileTime = SwapUShort(de->FileTime);
	de->FileDate = SwapUShort(de->FileDate);
	de->Crc = SwapUint(de->Crc);
	de->CompressedSize = SwapUint(de->CompressedSize);
	de->UncompressedSize = SwapUint(de->UncompressedSize);
	de->FileNameLength = SwapUShort(de->FileNameLength);
	de->ExtraFieldLength = SwapUShort(de->ExtraFieldLength);
	de->FileCommentLength = SwapUShort(de->FileCommentLength);
	de->DiskNumberStart = SwapUShort(de->DiskNumberStart);
	de->InternalAttributes = SwapUShort(de->InternalAttributes);
	de->ExternalAttributes = SwapUint(de->ExternalAttributes);
	de->HeaderOffset = SwapUint(de->HeaderOffset);
};

BOOL PkZip::FileEntryExists(string FileName) {
	
	// Loop through all our dir entries to find this filename
	for(int x = 0; x < endLocator.EntriesInDirectory; x++)
		if(strcmp(dirEntries[x].FileName, FileName.c_str()) == 0)
			return TRUE;

	// Failed to find the file
	return FALSE;
};
HRESULT PkZip::ExtractEntry(string ExtractDirectory, string FileName) {

	// Loop through all our dir entries to find this filename
	for(int x = 0; x < endLocator.EntriesInDirectory; x++)
		if(strcmp(dirEntries[x].FileName, FileName.c_str()) == 0)
			return ExtractEntry(ExtractDirectory, &dirEntries[x]);

	// Failed to find the file
	return 1;
};
HRESULT PkZip::ExtractEntry(string ExtractDirectory, DirEntry* dirEntry) {
		
	// Setup our final file path
	string finalPath = ExtractDirectory + dirEntry->FileName;
	finalPath = str_replaceallA(finalPath, "/", "\\");
	if(dirEntry->ExternalAttributes == 0x10 || dirEntry->ExternalAttributes == 0x30){
		CreateDirectory(finalPath.c_str(), NULL);
		return S_OK;
	}

	// Lets seek to the file record for this
	fseek(fHandle, dirEntry->HeaderOffset, SEEK_SET);
	fread(&fileEntry, sizeof(FileEntry), 1, fHandle);
	if(fileEntry.Signature != FileEntrySig){
		DebugMsg("PkZip::ExtractEntry", "Invalid FileEntry Signature");
		return 2;
	}
	SwapFileEntry(&fileEntry);
	fseek(fHandle, fileEntry.FileNameLength, SEEK_CUR); // Skip file name

	if(fileEntry.Compression != CompressionTypeStore && 
		fileEntry.Compression != CompressionTypeLZX) {
		DebugMsg("PkZip::ExtractEntry", "Compression type 0x%04X not supported", 
			fileEntry.Compression);
		return 3;
	}

	// Create our extracted file
	FILE* efHandle;
	fopen_s(&efHandle, finalPath.c_str(), "wb+");

	// Now start decompressing the data
	CRC_32 crc;
	DWORD crcVal = 0xFFFFFFFF;
	byte buffer[0x8000];
	uint sizeLeft = fileEntry.UncompressedSize;
	uint tempSize = 0;
	switch(fileEntry.Compression) {
		case CompressionTypeStore:
			while(sizeLeft > 0) {				
				tempSize = min(sizeLeft, 0x8000);
				fread(buffer, tempSize, 1, fHandle);
				crc.Calculate(buffer, tempSize, crcVal);
				fwrite(buffer, tempSize, 1, efHandle);
				sizeLeft -= tempSize;
			}
			break;
		case CompressionTypeLZX:
			// Create decompression ctx
			XMEMDECOMPRESSION_CONTEXT decompCtx = NULL;
			if(XMemCreateDecompressionContext(XMEMCODEC_DEFAULT, NULL, 
				0, &decompCtx) != 0) {
					fclose(efHandle);
					DebugMsg("PkZip::ExtractEntry", "Failed to create decompression context");
					return 4;
			}

			// Decompress our blocks
    		uint compressedBlockSize = 0;
			byte decompressBuffer[0x8000];
			while(sizeLeft > 0) {

				// Calculate block decompress size
				tempSize = sizeLeft > 0x8000 ? 0x8000 : sizeLeft;

				// Read compressed size and data
				fread(&compressedBlockSize, sizeof(uint), 1, fHandle);
				fread(buffer, compressedBlockSize, 1, fHandle);
				
				// Decompress data
				if(XMemDecompress(decompCtx, decompressBuffer, (SIZE_T*)&tempSize, 
					buffer, compressedBlockSize) != 0){
					fclose(efHandle);
					DebugMsg("PkZip::ExtractEntry", "Failed to decompress data");
					return 5;
				}

				// Write decompressed data to output file
				crc.Calculate(decompressBuffer, tempSize, crcVal);
				fwrite(decompressBuffer, tempSize, 1, efHandle);
			}

			// release decompression ctx
			XMemDestroyDecompressionContext( decompCtx );
			break;
	}

	// Close our extracted file
	fclose(efHandle);

	// Check our CRC32 to see if it matches
	if(fileEntry.Crc != (crcVal ^ 0xFFFFFFFF)) {
		DebugMsg("PkZip::ExtractEntry", "Calculated CRC32 does not match");	
		return 6;
	}

	// All done
	return S_OK;
};
HRESULT PkZip::ExtractAllEntries(string ExtractDirectory) {

	DebugMsg("TEST", "%s", ExtractDirectory.c_str());
	// Loop through all our dir entries
	for(int x = 0; x < endLocator.EntriesInDirectory; x++)
		if(ExtractEntry(ExtractDirectory, &dirEntries[x]) != S_OK) {
			DebugMsg("PkZip::ExtractAllEntries", "Failed to extract file \"%s\"",
				dirEntries[x].FileName);
			return 1;
		}

	return S_OK;
};