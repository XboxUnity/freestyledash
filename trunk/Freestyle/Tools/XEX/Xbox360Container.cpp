#include "stdafx.h"
#include "Xbox360Container.h"
#include "../Generic/xboxtools.h"
#include "../Generic/KernelExports.h"

BOOL XeKeysPkcs1VerifyCustom(const QWORD* Signature, const BYTE* Hash, const XECRYPT_RSA* Key) {

	// Check key modulus size
	if(Key->cqw * 8 > 0x200)
		return FALSE;

	// Swap our signature
	QWORD sigBuffer[0x20];
	XeCryptBnQw_SwapDwQwLeBe(Signature, sigBuffer, Key->cqw);

	// Decrypt our signature with our public key
	if(!XeCryptBnQwNeRsaPubCrypt(sigBuffer, sigBuffer, Key))
		return FALSE;

	// Swap our shit again
	XeCryptBnQw_SwapDwQwLeBe(sigBuffer, sigBuffer, Key->cqw);

	// Now actually verify our signature
	return XeCryptBnDwLePkcs1Verify(Hash, (BYTE*)sigBuffer, Key->cqw * 8);
}

BOOL XeKeysVerifyRSASignatureCustom(DWORD KeyIndex, const QWORD* Signature, const BYTE* Hash) {
	
	// Check our key
	DWORD keyNum = 0x39;
	if(KeyIndex == 0) keyNum = 0x39;
	else if(KeyIndex == 1) keyNum = 0x3D;
	else if(KeyIndex == 2) keyNum = 0x3F;
	else if(KeyIndex == 0) keyNum = 0;
	else return FALSE;

	// Get our key
	XECRYPT_RSAPUB_2048 rsaKey; 
	DWORD keySize = sizeof(XECRYPT_RSAPUB_2048);
	if(XeKeysGetKey(keyNum, &rsaKey, &keySize) < 0)
		return FALSE; // Failed to get our key =/

	// Verify our signature
	return XeKeysPkcs1VerifyCustom(Signature, Hash, (XECRYPT_RSA*)&rsaKey);
}

Xbox360Container::Xbox360Container(){
	pContainerHeader = (XCONTENT_HEADER*)malloc(sizeof(XCONTENT_HEADER));
	pContainerMetaData = (XCONTENT_METADATA*)malloc(sizeof(XCONTENT_METADATA));

}
Xbox360Container::~Xbox360Container(){
	if(pContainerHeader != NULL) free(pContainerHeader);
	if(pContainerMetaData != NULL) free(pContainerMetaData);
}

HRESULT Xbox360Container::OpenContainer(string ContainerPath){

	// Set some basic values
	isMounted = FALSE;
	containerPath = ContainerPath;
	ZeroMemory(pContainerHeader, sizeof(XCONTENT_HEADER));
	ZeroMemory(pContainerMetaData, sizeof(XCONTENT_METADATA));

	// Check if not a directory
	if (GetFileAttributes(ContainerPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
		return 1;
	}

	// Check if the file exists first
	if(!FileExists(containerPath)) {
		return 2;
	}

	// Open our xbe and begin to read it
	fopen_s(&fHandle, containerPath.c_str(), "rb");

	// Check for valid magic
	XCONTENT_SIGNATURE_TYPE type;
	fread(&type, sizeof(XCONTENT_SIGNATURE_TYPE), 1, fHandle);
	if(type != CONSOLE_SIGNED && type != LIVE_SIGNED && 
		type != PIRS_SIGNED){
		fclose(fHandle);
		return 3;
	}

	// Its valid so lets seek back and read the structure
	fseek(fHandle, 0, SEEK_SET);
	fread(pContainerHeader, sizeof(XCONTENT_HEADER), 1, fHandle);
	fread(pContainerMetaData, sizeof(XCONTENT_METADATA), 1, fHandle);

	// Verify our signature now to get the console type
	BYTE digest[0x14];
	XeCryptSha((BYTE*)pContainerHeader->LicenseDescriptors, 0x118,
		NULL, 0, NULL, 0, digest, 0x14);
	

	BOOL validSignature = FALSE;
	if(type == CONSOLE_SIGNED){
		isRetailCon = pContainerHeader->Signature.ConsoleSignature.Cert.ConsoleType == CONSOLETYPE_RETAIL ? TRUE : FALSE;
	}else if(type == LIVE_SIGNED){
		isRetailCon = XeKeysVerifyRSASignatureCustom(1, 
			(QWORD*)pContainerHeader->Signature.ContentSignature.Signature, digest );
	}else if(type == PIRS_SIGNED){
		isRetailCon = XeKeysVerifyRSASignatureCustom(0, 
			(QWORD*)pContainerHeader->Signature.ContentSignature.Signature, digest);
	}

	// All done
	return S_OK;
}
HRESULT Xbox360Container::CloseContainer(){
	fclose(fHandle);
	return S_OK;
}
wstring Xbox360Container::GetTitleName() {
	wstring szTitleName = pContainerMetaData->TitleName;
	return szTitleName;
}
HRESULT Xbox360Container::GetTitleName(string* TitleName, int* Length) {
	*TitleName = wchartochar(pContainerMetaData->TitleName, 0x40);
	*Length = 0x40;
	return S_OK;
}
wstring Xbox360Container::GetPublisher() {
	wstring szPublisher = pContainerMetaData->Publisher;
	return szPublisher;
}
HRESULT Xbox360Container::GetPublisher(string* Publisher, int* Length) {
	*Publisher = wchartochar(pContainerMetaData->Publisher, 0x40);
	*Length = 0x40;
	return S_OK;
}
HRESULT Xbox360Container::GetDisplayName(string* DisplayName, int* Length, int Language) {
	if(Language >= 0 && Language <= 8)
		*DisplayName = wchartochar(pContainerMetaData->DisplayName[Language], 0x80);
	else if(Language >= 9 && Language <= 10)
		*DisplayName = wchartochar(pContainerMetaData->DisplayNameEx[Language - 9], 0x80);
	else
		return 2; // Invalid language
	*Length = 0x80;
	return S_OK;
}
wstring Xbox360Container::GetDisplayName(int Language) {
	wstring szDisplayName = L"";
	if(Language >= 0 && Language <= 8)
		szDisplayName = pContainerMetaData->DisplayName[Language];
	else if(Language >= 9 && Language <= 10)
		szDisplayName = pContainerMetaData->DisplayNameEx[Language - 9];	
	return szDisplayName;
}
HRESULT Xbox360Container::GetDescription(string* Description, int* Length, int Language) {
	if(Language >= 0 && Language <= 8)
		*Description = wchartochar(pContainerMetaData->Description[Language], 0x80);
	else if(Language >= 9 && Language <= 10)
		*Description = wchartochar(pContainerMetaData->DescriptionEx[Language - 9], 0x80);
	else
		return 2; // Invalid language
	*Length = 0x80;
	return S_OK;
}
wstring Xbox360Container::GetDescription(int Language) {
	wstring szDesciption = L"";
	if(Language >= 0 && Language <= 8)
		szDesciption = pContainerMetaData->Description[Language];
	else if(Language >= 9 && Language <= 10)
		szDesciption = pContainerMetaData->DescriptionEx[Language - 9];
	return szDesciption;
}
HRESULT Xbox360Container::GetTitleId(DWORD* TitleId) {
	*TitleId = pContainerMetaData->ExecutionId.TitleID;
	return S_OK;
}
HRESULT Xbox360Container::GetMediaId(u32* MediaId) {
	*MediaId = pContainerMetaData->ExecutionId.MediaID;
	return S_OK;
}
HRESULT Xbox360Container::GetDiscNumbers(byte* CurrentDisc, byte* TotalDiscs) {
	*CurrentDisc = pContainerMetaData->ExecutionId.DiscNum;
	*TotalDiscs = pContainerMetaData->ExecutionId.DiscsInSet;
	return S_OK;
}
HRESULT Xbox360Container::GetContentType(DWORD* ContentType) {
	*ContentType = pContainerMetaData->ContentType;
	return S_OK;
}
HRESULT Xbox360Container::GetThumbnail(void** Data, int* Size) {
	*Data = malloc(pContainerMetaData->ThumbnailSize);
	*Size = pContainerMetaData->ThumbnailSize;
	memcpy(*Data, pContainerMetaData->Thumbnail, 
		pContainerMetaData->ThumbnailSize);
	return S_OK;
}
HRESULT Xbox360Container::GetTitleThumbnail(void** Data, int* Size) {
	*Data = malloc(pContainerMetaData->TitleThumbnailSize);
	*Size = pContainerMetaData->TitleThumbnailSize;
	memcpy(*Data, pContainerMetaData->TitleThumbnail, 
		pContainerMetaData->TitleThumbnailSize);
	return S_OK;
}
HRESULT Xbox360Container::Mount(string MountPath) {

	if(isMounted) UnMount();

	mountPath = MountPath;
	string drive = containerPath.substr(0, containerPath.find(":") + 1);
	string folder = containerPath.substr(containerPath.find(":") + 2);

	char drive_s[256];
	char folder_s[1024];

	strcpy_s(drive_s, 256, drive.c_str());	
	strcpy_s(folder_s, 1024, folder.c_str());

	HRESULT result = mountCon((char*)mountPath.c_str(), drive_s, folder_s);
	isMounted = result == 0;

	return result;
}
HRESULT Xbox360Container::UnMount(bool bSaveChanges) {
	if(isMounted) {
		isMounted = false;	
		return unmountCon((char*)mountPath.c_str(), bSaveChanges);
	}
	return S_OK;
}
void Xbox360Container::LaunchGame() {

	if(!isMounted)
		if(Mount("dice") != S_OK)
			return;

	string launchPath = mountPath.append(
		pContainerMetaData->ContentType == XCONTENTTYPE_XBOXTITLE ? 
		":\\default.xbe" : ":\\default.xex");

	if(FileExists(launchPath))
		XLaunchNewImage(launchPath.c_str(), 0);
}
HRESULT Xbox360Container::ExtractNXE(string destFilePath, DWORD nxeImage ) {

	if(pContainerMetaData->ContentType != XCONTENTTYPE_THEMATICSKIN)
		return 1;

	BOOL previouslyMounted = isMounted;
	if(!previouslyMounted)
		Mount("nxe");

	string mountpath = mountPath;

	if(nxeImage == NxeImageBackground)
	{
		string bgPath = mountpath.append(":\\nxebg.jpg");
		if(FileExists(bgPath)){
			CopyFile(bgPath.c_str(), destFilePath.c_str(), false);
			//ConvertImageToDXT5(destFilePath);
			#ifdef _USING_DXT5
				ConvertImageFileToDXT5(bgPath, destFilePath);	
			#else if
				ConvertImageFileToDXT1(bgPath, destFilePath);	
			#endif
		}
	}

	if(nxeImage == NxeImageSlot)
	{
		string slotPath = mountpath.append(":\\nxeslot.jpg");
		if(FileExists(slotPath)){
			//CopyFile(slotPath.c_str(), destFilePath.c_str(), false);
			#ifdef _USING_DXT5
				ConvertImageFileToDXT5(slotPath, destFilePath);
			#else if
				ConvertImageFileToDXT1(slotPath, destFilePath);
			#endif
		}
	}

	if(nxeImage == NxeImageBanner)
	{
		string bannerPath = mountpath.append(":\\nxebanner.dds");
		if(FileExists(bannerPath)){
			CopyFile(bannerPath.c_str(), destFilePath.c_str(), false);
		}
	}

	if(nxeImage == NxeImageBoxart)
	{
		string boxartPath = mountpath.append(":\\nxeboxart.dds");
		if(FileExists(boxartPath)){
			CopyFile(boxartPath.c_str(), destFilePath.c_str(), false);
		}
	}

	if(nxeImage == NxeImageScreenshot1)
	{
		string ss1Path = mountpath.append(":\\nxess1.dds");
		if(FileExists(ss1Path)){
			CopyFile(ss1Path.c_str(), destFilePath.c_str(), false);
		}
	}

	if(nxeImage == NxeImageScreenshot2)
	{
		string ss2Path = mountpath.append(":\\nxess2.dds");
		if(FileExists(ss2Path)){
			CopyFile(ss2Path.c_str(), destFilePath.c_str(), false);
		}
	}

	if(!previouslyMounted)
		UnMount();

	return S_OK;
}

HRESULT Xbox360Container::WriteDWORDToOffset(DWORD Value, fstream::pos_type Offset)
{
	HRESULT result = S_OK;
	bool wasOpen = false;
	if (fHandle)
	{
		 wasOpen = true;
		 CloseContainer();
	}
	
	string theFile(containerPath);
				
	DebugMsg("Xbox360Container", "opening file: %s", theFile.c_str());
	
	fstream ContainerFile;
	ContainerFile.open(theFile.c_str(), fstream::in | fstream::out | fstream::binary );
	if (ContainerFile.is_open())
	{
		DebugMsg("Xbox360Container", "writing DWORD 0x%08x to offset 0x%x", Value, Offset);
		ContainerFile.seekp(0x354);
		ContainerFile.write((char*)&Value, sizeof(DWORD));
		DebugMsg("Xbox360Container", "DONE: writing value");
		ContainerFile.close();
	}
	else
	{
		DebugMsg("Xbox360Container", "FAILED: opening file");
		result = -1;
	}
	

	if (wasOpen)
	{
		 OpenContainer(containerPath);
	}	

	return result;
}

HRESULT Xbox360Container::ReHash()
{
	HRESULT result = S_OK;
	bool wasOpen = false;
	if (fHandle)
	{
		wasOpen = true;
		CloseContainer();
	}
	
	string theFile(containerPath);
				
	char* SHA1Buffer[0xACBC];
	char* SHA1Digest[20];

	DebugMsg("Xbox360Container", "opening file: %s", theFile.c_str());
	
	fstream ContainerFile;
	ContainerFile.open(theFile.c_str(), fstream::in | fstream::out | fstream::binary );
	if (ContainerFile.is_open())
	{
		DebugMsg("Xbox360Container", "rehashing file");
					
		ContainerFile.flush();
		ContainerFile.seekg(0x344);
		ContainerFile.read((char*)SHA1Buffer,0xACBC);
		XeCryptSha((const BYTE*)SHA1Buffer, 0xACBC, NULL,0, NULL,0, (BYTE*)SHA1Digest, 20);
		ContainerFile.seekp(0x32C);
		ContainerFile.write((char*)SHA1Digest, 20);
		ContainerFile.close();
		DebugMsg("Xbox360Container", "DONE: rehashing file");
	}	
	else
	{
		DebugMsg("Xbox360Container", "FAILED: opening file");
		result = -1;
	}
	
	if (wasOpen)
	{
		 OpenContainer(containerPath);
	}	

	return result;
}

HRESULT Xbox360Container::ConvertNXE2GOD(string& ItemDirectory)
{
	HRESULT result = S_OK;

	DebugMsg("Xbox360Container", "checking if container is NXE installed game");
	if (pContainerMetaData->ContentType == XCONTENTTYPE_INSTALLED_XBOX360TITLE)
	{
		DebugMsg("Xbox360Container", "DONE: checking if container is NXE installed game");
		
		bool wasOpen = false;
		if (fHandle)
		{
			 CloseHandle(fHandle);
		wasOpen = true;
			 CloseContainer();

		}

		string theFullPath(containerPath);
		
		string theFile = containerPath.substr(containerPath.find_last_of("\\") + 1);
		string theFolder = containerPath.substr(0, containerPath.find_last_of("\\"));
		string theNewFolder = theFolder;
		
		if (theNewFolder.substr(theNewFolder.length() - 9) == "\\00004000")
		{
			theNewFolder.replace(theNewFolder.length() - 8, 8, "00007000");
			DebugMsg("Xbox360Container", "moving folder 00004000 -> 00007000");
			if (!(::MoveFile(theFolder.c_str(), theNewFolder.c_str())))
			{
				DebugMsg("Xbox360Container", "FAILED: moving folder 00004000 -> 00007000: error %d", GetLastError());
				return -1;
			}
			DebugMsg("Xbox360Container", "DONE: moving folder 00004000 -> 00007000");
			theFullPath = theNewFolder + theFile;
			ItemDirectory = theNewFolder;
		}
		
		DebugMsg("Xbox360Container", "opening file: %s", theFullPath.c_str());
		fstream ContainerFile;
		ContainerFile.open(theFullPath.c_str(), fstream::in | fstream::out | fstream::binary );
		if (ContainerFile.is_open())
		{
			DebugMsg("Xbox360Container", "converting to GOD");
			ContainerFile.seekp(0x340);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0xAD);
			ContainerFile.put(0x0E);
			ContainerFile.seekp(0x346);
			ContainerFile.put(0x70);
			ContainerFile.seekp(0x379);
			ContainerFile.put(0x24);
			ContainerFile.put(0x5);
			ContainerFile.put(0x5);
			ContainerFile.put(0x11);
			ContainerFile.seekp(0x3FD);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.put(0x0);
			ContainerFile.seekp(0x22c);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.put(0xFF);
			ContainerFile.seekp(0);
			ContainerFile.put(0x4C);
			ContainerFile.put(0x49);
			ContainerFile.put(0x56);
			ContainerFile.put(0x45);
			DebugMsg("Xbox360Container", "DONE: converting to GOD");
			ContainerFile.close();

			result = ReHash();
		}
		else
		{
			DebugMsg("Xbox360Container", "FAILED: opening file");
			result = -2;
		}
		

		if (wasOpen)
		{
			 OpenContainer(containerPath);
		}


	}
	else
	{
		result = -3;
		DebugMsg("Xbox360Container", "FAILED: checking if container is NXE installed game");
	}


	return result;
}