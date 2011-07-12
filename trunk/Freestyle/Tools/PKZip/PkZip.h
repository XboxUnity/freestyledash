#pragma once

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "../Generic/CRC_32.h"

typedef unsigned short ushort;
typedef unsigned int   uint;

class PkZip
{
private:

	#define CompressionTypeStore 0x0000
	#define CompressionTypeLZX   0x0015

	#define DirEntrySig			 0x504B0102
	#define FileEntrySig		 0x504B0304
	#define EndLocatorSig		 0x504B0506

#pragma pack(1)
	struct EndLocator {
		uint     Signature;
		ushort   DiskNumber;
		ushort   StartDiskNumber;
		ushort   EntriesOnDisk;
		ushort   EntriesInDirectory;
		uint     DirectorySize;
		uint     DirectoryOffset;
		ushort   CommentLength;
	};

	struct DirEntry {
		uint     Signature;
		ushort   VersionMadeBy;
		ushort   VersionToExtract;
		ushort   Flags;
		ushort   Compression;
		ushort   FileTime;
		ushort   FileDate;
		uint     Crc;
		uint     CompressedSize;
		uint     UncompressedSize;
		ushort   FileNameLength;
		ushort   ExtraFieldLength;
		ushort   FileCommentLength;
		ushort   DiskNumberStart;
		ushort   InternalAttributes;
		uint     ExternalAttributes;
		uint     HeaderOffset;
		char*    FileName;
	};

	struct FileEntry {
		uint     Signature;
		ushort   Version;
		ushort   Flags;
		ushort   Compression;
		ushort   FileTime;
		ushort   FileDate;
		uint     Crc;
		uint     CompressedSize;
		uint     UncompressedSize;
		ushort   FileNameLength;
		ushort   ExtraFieldLength;
	};
	#pragma pack()

	FILE*		fHandle;
	EndLocator  endLocator;
	DirEntry*   dirEntries;
	FileEntry   fileEntry;

	HRESULT ExtractEntry(string ExtractDirectory, DirEntry* dirEntry);
	void SwapEndLocator(EndLocator* el);
	void SwapFileEntry(FileEntry* fe);
	void SwapDirEntry(DirEntry* de);

public:
	PkZip(void){};
	~PkZip(void){};
	HRESULT Open(string ZipPath);
	void Close();
	HRESULT ExtractEntry(string ExtractDirectory, string FileName);
	HRESULT ExtractAllEntries(string ExtractDirectory);
	BOOL FileEntryExists(string FileName);
};