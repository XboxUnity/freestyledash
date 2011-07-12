#include "stdafx.h"
#include "XboxExecutable.h"

DWORD SwapDword(DWORD Value){
	return (DWORD)(((Value & 0xFF000000) >> 24) | ((Value & 0xFF0000) >> 8) | 
		((Value & 0xFF00) << 8) | ((Value & 0xFF) << 24));
}

WORD SwapWord(WORD Value){
	return (WORD)(((Value & 0xFF00) >> 8) | ((Value & 0xFF) << 8));
}

void SwapWStr(PWSTR Str) {
	for (int i = 0; i < 40; i++)
		Str[i] = (WCHAR)SwapWord((WORD)Str[i]);
}

HRESULT XboxExecutable::OpenExecutable(string ExecutablePath){
	
	// Check if the file exists first
	if(!FileExists(ExecutablePath)) {
		return 1;
	}

	// Open our xbe and begin to read it
	fopen_s(&fHandle, ExecutablePath.c_str(), "rb");
	fread(&Header, sizeof(XbeHeader), 1, fHandle);

	// Make sure its a valid xbe
	if(Header.Magic != 0x58424548){
		fclose(fHandle);
		return 2;
	}

	// Gotta byte swap all the values now
	Header.Magic = SwapDword(Header.Magic);
	Header.BaseAddress = SwapDword(Header.BaseAddress);
	Header.HeaderSize = SwapDword(Header.HeaderSize);
	Header.ImageSize = SwapDword(Header.ImageSize);
	Header.XbeHeaderSize = SwapDword(Header.XbeHeaderSize);
	Header.Timestamp = SwapDword(Header.Timestamp);
	Header.CertPtr = SwapDword(Header.CertPtr);
	Header.NumSections = SwapDword(Header.NumSections);
	Header.SectionsPtr = SwapDword(Header.SectionsPtr);
	Header.InitFlags = SwapDword(Header.InitFlags);
	Header.EntryPoint = SwapDword(Header.EntryPoint);
	Header.TlsDirPtr = SwapDword(Header.TlsDirPtr);
	Header.StackCommit = SwapDword(Header.StackCommit);
	Header.HeapReserve = SwapDword(Header.HeapReserve);
	Header.HeapCommit = SwapDword(Header.HeapCommit);
	Header.PeBaseAddress = SwapDword(Header.PeBaseAddress);
	Header.PeImageSize = SwapDword(Header.PeImageSize);
	Header.PeChecksum = SwapDword(Header.PeChecksum);
	Header.PeTimestamp = SwapDword(Header.PeTimestamp);
	Header.PcExePathPtr = SwapDword(Header.PcExePathPtr);
	Header.PcExeFilenamePtr = SwapDword(Header.PcExeFilenamePtr);
	Header.PcExeFilenameUnicodePtr = SwapDword(Header.PcExeFilenameUnicodePtr);
	Header.KernelThunkTable = SwapDword(Header.KernelThunkTable);
	Header.DebugImportTable = SwapDword(Header.DebugImportTable);
	Header.NumLibraries = SwapDword(Header.NumLibraries);
	Header.LibrariesPtr = SwapDword(Header.LibrariesPtr);
	Header.KernelLibPtr = SwapDword(Header.KernelLibPtr);
	Header.XapiLibPtr = SwapDword(Header.XapiLibPtr);
	Header.LogoBitmap = SwapDword(Header.LogoBitmap);
	Header.LogoBitmapSize = SwapDword(Header.LogoBitmapSize);

	// Now we can seek to and read our certificate
	fseek(fHandle, Header.CertPtr - Header.BaseAddress, SEEK_SET);
	fread(&Certificate, sizeof(XbeCertificate), 1, fHandle);

	// Now we gotta byte swap =/
	Certificate.Size = SwapDword(Certificate.Size);
	Certificate.Timestamp = SwapDword(Certificate.Timestamp);
	Certificate.TitleId = SwapDword(Certificate.TitleId);
	SwapWStr((WCHAR*)Certificate.TitleName);
	for(int x = 0; x < 16; x++)
		Certificate.AlternateTitleIds[x] = SwapDword(Certificate.AlternateTitleIds[x]);
	Certificate.MediaTypes = SwapDword(Certificate.MediaTypes);
	Certificate.GameRegion = SwapDword(Certificate.GameRegion);
	Certificate.GameRating = SwapDword(Certificate.GameRating);
	Certificate.DiskNumber = SwapDword(Certificate.DiskNumber);
	Certificate.Version = SwapDword(Certificate.Version);

	
	// Now we can read our sections
	Sections = new XbeSection[Header.NumSections];
	fseek(fHandle, Header.SectionsPtr - Header.BaseAddress, SEEK_SET);
	fread(Sections, sizeof(XbeSection), Header.NumSections, fHandle);

	// Now we gotta loop and byte swap each =/
	for(int x = 0; x < (int)Header.NumSections; x++){
		Sections[x].Flags = SwapDword(Sections[x].Flags);
		Sections[x].VirtualAddress = SwapDword(Sections[x].VirtualAddress);
		Sections[x].VirtualSize = SwapDword(Sections[x].VirtualSize);
		Sections[x].FileAddress = SwapDword(Sections[x].FileAddress);
		Sections[x].FileSize = SwapDword(Sections[x].FileSize);
		Sections[x].SectionNamePtr = SwapDword(Sections[x].SectionNamePtr);
		Sections[x].SectionReferenceCount = SwapDword(Sections[x].SectionReferenceCount);
		Sections[x].HeadReferenceCount = SwapDword(Sections[x].HeadReferenceCount);
		Sections[x].TailReferenceCount = SwapDword(Sections[x].TailReferenceCount);
	}

	// Finally finished =D
	return S_OK;
}

HRESULT XboxExecutable::CloseExecutable(){
	fclose(fHandle);
	if(Sections != NULL) delete Sections;
	return S_OK;
}

wstring XboxExecutable::GetTitleName() { return wstring(Certificate.TitleName); }
HRESULT XboxExecutable::GetTitleName(string * titleName, int * Length)
{
	*titleName = wchartochar(Certificate.TitleName, 40);
	*Length = 40;

	return S_OK;
}

DWORD XboxExecutable::GetTitleId() { return Certificate.TitleId; }
HRESULT XboxExecutable::GetTitleId(DWORD * titleId)
{
	*titleId = Certificate.TitleId;
	return S_OK;
}

DWORD XboxExecutable::GetMediaId() { return Certificate.MediaTypes; }
HRESULT XboxExecutable::GetMediaId(DWORD * mediaTypes)
{
	*mediaTypes = Certificate.MediaTypes;
	return S_OK;
}

DWORD XboxExecutable::GetCurrentDisc() { return Certificate.DiskNumber; }
HRESULT XboxExecutable::GetCurrentDisc(DWORD * currentDisc)
{
	*currentDisc = Certificate.DiskNumber;
	return S_OK;
}

HRESULT XboxExecutable::GetGameThumbnail(void** Data, int* Size){
	
	CHAR ImageName[9] = { 0x24, 0x24, 0x58, 0x54, 0x49, 0x4D, 0x41, 0x47, 0x45 };  // $$XTIMAGE
	CHAR TempName[9];

	// Gotta loop through all our sections
	for(int x = 0; x < (int)Header.NumSections; x++){
		fseek(fHandle, Sections[x].SectionNamePtr - Header.BaseAddress, SEEK_SET);
		fread(TempName, sizeof(CHAR), 9, fHandle);
		if(memcmp(ImageName, TempName, 9) != 0)
			continue;

		// Its a match so lets goto and read the data
		byte* tempData = (byte*)malloc(Sections[x].FileSize);
		fseek(fHandle, Sections[x].FileAddress, SEEK_SET);
		fread(tempData, Sections[x].FileSize, 1, fHandle);

		// Apply our xpro header and byte swap
		if(((XproHeader*)tempData)->Magic != 0x58505230){	
			free(tempData);
			return 1;
		}

		// Get our data start and size
		((XproHeader*)tempData)->FileSize = SwapDword(((XproHeader*)tempData)->FileSize);
		((XproHeader*)tempData)->DataStart = SwapDword(((XproHeader*)tempData)->DataStart);

		// Now lets get our dxt data finally..
		int dataSize = ((XproHeader*)tempData)->FileSize - ((XproHeader*)tempData)->DataStart;
		*Data = malloc(dataSize + 0x80); // DXT1 Header is 128 bytes
		*Size = dataSize + 0x80;
		memcpy(*Data, Dxt1Header, 0x80);
		memcpy((byte*)*Data + 0x80, tempData + ((XproHeader*)tempData)->DataStart, dataSize);
		
		// All done
		free(tempData);
		return S_OK;
	}

	// Could not find it =/
	return 2;
}