#include "stdafx.h"
#include "XeXtractor.h"

BOOL XeCryptBnQwBeSigVerifyCustom(XECRYPT_SIG * pSig, BYTE * pbHash, BYTE * pbSalt, XECRYPT_RSA * pRsa) {

	// Check our size and exponent
	if(pRsa->cqw != 0x20) return FALSE;
	if(pRsa->dwPubExp != 3 && pRsa->dwPubExp != 0x10001) return FALSE;

	QWORD inverse = XeCryptBnQwNeModInv(((XECRYPT_RSAPUB_2048*)pRsa)->aqwM[0]);
	QWORD signatureBuffer[0x20];
	XeCryptBnQw_Copy((QWORD*)pSig, signatureBuffer, 0x20);

	DWORD exponent = pRsa->dwPubExp;
	while(exponent > 1) {
		exponent >>= 1;	
		XeCryptBnQwNeModMul(signatureBuffer, signatureBuffer, 
			signatureBuffer, inverse, ((XECRYPT_RSAPUB_2048*)pRsa)->aqwM, 0x20);		
	}

	XeCryptBnQwNeModMul(signatureBuffer, (QWORD*)pSig,
		(QWORD*)pSig, inverse, ((XECRYPT_RSAPUB_2048*)pRsa)->aqwM, 0x20);

	XeCryptBnQwBeSigFormat((XECRYPT_SIG*)signatureBuffer, pbHash, pbSalt);

	return memcmp(pSig, signatureBuffer, 0x20 * 0x08) == 0; 
}
XeXtractor::XeXtractor() {
	isOpen = FALSE;
	isRetailXex = FALSE;
	FullHeader = NULL;
	Header = NULL;
	DirectoryEntries = NULL;
	SecurityInfo = NULL;
	PageInfo = NULL;
 }
XeXtractor::~XeXtractor() {
	CloseXex();
}
HRESULT XeXtractor::OpenXex(string XexPath) {

	// Close if already open
	CloseXex();

	// Check if the file exists first
	if(!FileExists(XexPath)) {
		return 1;
	}

	// Open our xex and begin to read it
	fopen_s(&fHandle, XexPath.c_str(), "rb");
	IMAGE_XEX_HEADER tempHeader;
	fread(&tempHeader, sizeof(IMAGE_XEX_HEADER), 1, fHandle);

	// Make sure its a valid xex
	if(tempHeader.Magic != IMAGE_XEX_HEADER_MAGIC){
		fclose(fHandle);	
		return 2;
	}

	// Its a valid xex so lets go back and read the full header
	FullHeader = (PBYTE)malloc(tempHeader.SizeOfHeaders);
	fseek(fHandle, 0, SEEK_SET);
	fread(FullHeader, tempHeader.SizeOfHeaders, 1, fHandle);
	Header = (PIMAGE_XEX_HEADER)FullHeader;
	DirectoryEntries = (PIMAGE_XEX_DIRECTORY_ENTRY)(Header + 1);
	SecurityInfo = (PXEX_SECURITY_INFO)(FullHeader + Header->SecurityInfo);
	PageInfo = (PHV_PAGE_INFO)(SecurityInfo + 1);

	// Lets do some basic verficiation to "try" and detect xex type
	BYTE digest[0x14];
	XeCryptRotSumSha((byte*)&SecurityInfo->ImageInfo.InfoSize, 
		SecurityInfo->ImageInfo.InfoSize - 0x100, 0, 0, digest, 0x14);

	XECRYPT_RSAPUB_2048 pubRsaKey; DWORD keySize = 0x110;
	XeKeysGetKey(0x39, &pubRsaKey, &keySize);		

	// Verify our signature -- if the RSA signatures do not match, it is assumed devkit xex
	BOOL validSignature = XeCryptBnQwBeSigVerifyCustom(
		(XECRYPT_SIG*)SecurityInfo->ImageInfo.Signature, 
		digest, (BYTE*)XexSalt, (XECRYPT_RSA*)&pubRsaKey);

	// if the signature is not valid- we still need to check for a zerostring RSA signature
	if(!validSignature) {
		validSignature = TRUE;
		for(int x = 0; x < 0x100; x++)
			if(SecurityInfo->ImageInfo.Signature[x] != 0x00) {
				validSignature = FALSE; break;
			}
	}

	// This will let us know what public key we used, a devkit or a retail
	DWORD consoleType = 0;
	XeKeysGetConsoleTypeCustom(&consoleType);
	if(consoleType == CONSOLETYPE_RETAIL && validSignature)
		isRetailXex = TRUE; // Its a retail and a valid signature
	else if(consoleType == CONSOLETYPE_RETAIL && !validSignature)
		isRetailXex = FALSE; // Its a retail and not a valid signature
	else if(consoleType != CONSOLETYPE_RETAIL && validSignature)
		isRetailXex = FALSE; // Its not a retail and a valid signature
	else if(consoleType != CONSOLETYPE_RETAIL && !validSignature)
		isRetailXex = TRUE; // Its not a retail but also not a valid signature	

	// Our file is now open
	isOpen = TRUE;
	return S_OK;
}
HRESULT XeXtractor::CloseXex() {
	if(isOpen) {
		// Free all our header stuff
		Header = NULL;
		DirectoryEntries = NULL;
		SecurityInfo = NULL;
		PageInfo = NULL;
		free(FullHeader);

		// Close our file
		fclose(fHandle);
		isOpen = FALSE;
	}
	return S_OK;
}
HRESULT XeXtractor::GetOptionalHeader(DWORD OptHeaderKey, PVOID* Data) {

	// Loop through all our headers and try and find the key
	DWORD x; BOOL found = FALSE;
	for(x = 0; x < Header->HeaderDirectoryEntryCount; x++) {
		if(DirectoryEntries[x].Key == OptHeaderKey) {
			found = TRUE; break;
		}
	}

	// Make sure we found a key
	if(!found) return 1;

	// We found our key so lets get our data size
	DWORD dataSize = OptHeaderKey & 0xFF;
	
	// If our data size is 1 dword then its stored in our data
	if(dataSize == 1){
		*Data = &DirectoryEntries[x].Value;
		return S_OK;
	}

	// Its stored in a optional header
	*Data = FullHeader + DirectoryEntries[x].Value;
	return S_OK;
}
HRESULT XeXtractor::CalculateDecryptKey(PBYTE DecryptKey){
		
	// Setup our ctx and iv
	XECRYPT_AES_STATE ctx; ZeroMemory(&ctx, sizeof(XECRYPT_AES_STATE));
	BYTE iv[16]; ZeroMemory(iv, 16);
	
	// Set our key to use
	if(isRetailXex) XeCryptAesKey(&ctx, (BYTE*)RetailKey);
	else XeCryptAesKey(&ctx, (BYTE*)DevkitKey);

	// Get our decrypt key
	XeCryptAesCbc(&ctx, SecurityInfo->ImageInfo.ImageKey, 16, DecryptKey, iv, 0);

	return S_OK;
}
HRESULT XeXtractor::GetBaseFile(PBYTE* Basefile, PDWORD Size){
	
	// Get our base file format
	PXEX_FILE_DATA_DESCRIPTOR descriptor = NULL;
	if(GetOptionalHeader(DIRECTORY_KEY_BASEFILE_DESCRIPTOR, 
		(PVOID*)&descriptor) != S_OK) return 1;

	// Setup our result
	HRESULT result = S_OK;

	// Alocate our space
	*Size = SecurityInfo->ImageSize;
	*Basefile = (PBYTE)malloc(SecurityInfo->ImageSize);
	ZeroMemory(*Basefile, SecurityInfo->ImageSize);
	PBYTE BasefilePos = *Basefile;

	// Setup our decrypt key incase we need it
	BYTE key[16];
	CalculateDecryptKey(key);
	XECRYPT_AES_STATE ctx;
	BYTE iv[16]; ZeroMemory(iv, 16);
	XeCryptAesKey(&ctx, key);

	// Seek to our data location
	DWORD sizeLeft = *Size;
	fseek(fHandle, Header->SizeOfHeaders, SEEK_SET);

	// Get our format
	if(descriptor->Format == FILE_DATA_FORMAT_NOT_COMPRESSED){
		
		// Get our block count and ptr
		DWORD blockCount = (descriptor->Size - sizeof(XEX_FILE_DATA_DESCRIPTOR)) / 
			sizeof(XEX_RAW_DATA_DESCRIPTOR);
		PXEX_RAW_DATA_DESCRIPTOR rawBlocks = (PXEX_RAW_DATA_DESCRIPTOR)(descriptor + 1);

		// Loop through all our blocks
		for(DWORD x = 0; x < blockCount; x++){

			// Read our data into our basefile and decrypt if needed
			fread(BasefilePos, rawBlocks[x].DataSize, 1, fHandle);
			if(descriptor->Flags & FILE_DATA_FLAGS_ENCRYPTED)
				XeCryptAesCbc(&ctx, BasefilePos, rawBlocks[x].DataSize, BasefilePos, iv, 0);

			BasefilePos += (rawBlocks[x].DataSize + rawBlocks[x].ZeroSize);
			sizeLeft -= (rawBlocks[x].DataSize + rawBlocks[x].ZeroSize);
		}
	} else if(descriptor->Format == FILE_DATA_FORMAT_COMPRESSED) {

		// Get our base info and block info
		PXEX_COMPRESSED_DATA_DESCRIPTOR baseInfo = 
			(PXEX_COMPRESSED_DATA_DESCRIPTOR)(descriptor + 1);
		XEX_DATA_DESCRIPTOR block; 
		memcpy(&block, &baseInfo->FirstDescriptor, sizeof(XEX_DATA_DESCRIPTOR)); 			

		// Create decompression context
		DWORD maxSize = 0x8000; DWORD ldiCtx = 0xFFFFFFFF; 
		DWORD unknown = 0; LZX_DECOMPRESS lzx = {baseInfo->WindowSize, 1};
		VOID* allocate = malloc(0x23200 + baseInfo->WindowSize);
		if(LDICreateDecompression(&maxSize, &lzx, 0, 0, allocate, &unknown, &ldiCtx) != 0) {
			result = 2;	free(allocate);
			goto CLEANUP;
		}

		// Loop through and decompress
		while(result == S_OK && block.Size > 0){

			// Read our block and decrypt if we must
			PBYTE buffer = (PBYTE)malloc(block.Size);
			fread(buffer, block.Size, 1, fHandle);
			if(descriptor->Flags & FILE_DATA_FLAGS_ENCRYPTED)
				XeCryptAesCbc(&ctx, buffer, block.Size, buffer, iv, 0);

			// Verify our hash for this block
			BYTE digest[0x14];
			XeCryptSha(buffer, block.Size, NULL, 0, NULL, 0, digest, 0x14);
			if(memcmp(digest, block.DataDigest, 0x14) != 0) {
				result = 3; break;
			}

			// Copy over our next block descriptor
			memcpy(&block, buffer, sizeof(XEX_DATA_DESCRIPTOR));

			// Decompress all the sub blocks within this block
			PWORD subBlockSize = (PWORD)(buffer + sizeof(XEX_DATA_DESCRIPTOR));
			PBYTE subBlockData = (PBYTE)(subBlockSize + 1);
			while(*subBlockSize > 0){
				
				// Decompress our data
				DWORD decompressedSize = (sizeLeft < 0x8000) ? sizeLeft : 0x8000;
				if(LDIDecompress(ldiCtx, subBlockData, *subBlockSize,
					BasefilePos, &decompressedSize) != 0) {
					result = 4;	break;
				}

				// Setup the next block
				BasefilePos += decompressedSize;
				sizeLeft -= decompressedSize;
				subBlockSize = (PWORD)(subBlockData + *subBlockSize);
				subBlockData = (PBYTE)(subBlockSize + 1);				
			}
			
			// Free our temp block
			free(buffer);
		}

		// Clean up our buffers
		LDIDestroyDecompression(ldiCtx);			
		free(allocate);
	} else {
		result = 5;
	}

	// Verify our image now
	BasefilePos = *Basefile; BYTE pageDigest[0x14];
	PBYTE pageVerifyDigest = SecurityInfo->ImageInfo.ImageHash;
	DWORD pageSize = (SecurityInfo->ImageInfo.ImageFlags & IMAGE_FLAG_PAGE_SIZE_4KB) ? 0x1000 : 0x10000;
	for(DWORD x = 0; x < SecurityInfo->PageDescriptorCount; x++) {

		DWORD currentPageSize = PageInfo[x].PageDescription.Size * pageSize;
		XeCryptSha(BasefilePos, currentPageSize, 
			(PBYTE)&PageInfo[x], sizeof(HV_PAGE_INFO),
			NULL, 0, pageDigest, 0x14);

		if(memcmp(pageDigest, pageVerifyDigest, 0x14) != 0) {
			/*result = 6;*/ break;
		}

		BasefilePos += currentPageSize;
		pageVerifyDigest = PageInfo[x].DataDigest;
	}

	CLEANUP:
	if(result != S_OK) {
		free(*Basefile);
		*Basefile = NULL;
		*Size = 0;
	}

	return result;
}
HRESULT XeXtractor::ExtractResource(string ResrouceName, PBYTE* Resource, PDWORD Size){
	
	// Get our resource header
	PXEX_SECTION_INFO sectionInfo = NULL;
	if(GetOptionalHeader(DIRECTORY_KEY_SECTION, 
		(PVOID*)&sectionInfo) != S_OK) return 1;

	// Now lets try and find our resource name
	int resourceCount = (sectionInfo->Size - sizeof(DWORD)) / 
		sizeof(XEX_SECTION_HEADER);	bool found = false;
	PXEX_SECTION_HEADER resource = NULL;
	CHAR resTitle[9]; ZeroMemory(resTitle, 9);
	for(int x = 0; x < resourceCount; x++){
		memcpy(resTitle, sectionInfo->Section[x].SectionName, 8);
		if(strcmp(resTitle, ResrouceName.c_str()) == 0){
			resource = &sectionInfo->Section[x];
			found = true; break;
		}
	}
	if(!found) {
		return 2;
	}

	// Okay we got our resource info lets get our basefile
	PBYTE baseFile;	DWORD baseSize;
	if(GetBaseFile(&baseFile, &baseSize) != S_OK) { 
		return 3;
	}

	// Now lets allocate our resource and copy from our basefile
	*Resource = (PBYTE)malloc(resource->VirtualSize);
	*Size = resource->VirtualSize;

	PBYTE resourceData = baseFile + (resource->VirtualAddress - SecurityInfo->ImageInfo.LoadAddress);
	memcpy(*Resource, resourceData, resource->VirtualSize);

	// All done lets clean up
	free(baseFile);
	return S_OK;
}
HRESULT XeXtractor::ExtractTitleSPA(PBYTE* Resrouce, PDWORD Size){

	// Get our execution id
	PXEX_EXECUTION_ID executionId = NULL;
	if(GetOptionalHeader(DIRECTORY_KEY_EXECUTION_ID, 
		(PVOID*)&executionId) != S_OK) return 2;

	// Get a resource using our title id
	string spaId = sprintfa("%08X", executionId->TitleID);
	return ExtractResource(spaId, Resrouce, Size);
}
HRESULT XeXtractor::GetExecutionId(PXEX_EXECUTION_ID ExecutionId){

	// Get our execution id
	PXEX_EXECUTION_ID executionId = NULL;
	if(GetOptionalHeader(DIRECTORY_KEY_EXECUTION_ID, 
		(PVOID*)&executionId) != S_OK) return 1;

	// Copy it over
	memcpy(ExecutionId, executionId, sizeof(XEX_EXECUTION_ID));

	// Clean up
	return S_OK;
}
HRESULT XeXtractor::GetRating(BYTE RatingType, PBYTE Rating) {

	// Get our raitings
	PXEX_GAME_RATINGS ratings = NULL;
	if(GetOptionalHeader(DIRECTORY_KEY_GAME_RATINGS, 
		(PVOID*)&ratings) != S_OK) return 1;

	// Now get the proper one for our type
	*Rating = ratings->Ratings[RatingType];

	return S_OK;
}
HRESULT XeXtractor::PrintInfo() {

	// Display some debug information
	DebugMsg("XeXtractor", "Magic      : %08x", Header->Magic);
	DebugMsg("XeXtractor", "ModuleFlag : %08x", Header->ModuleFlags);
	DebugMsg("XeXtractor", "DataOffset : %08x", Header->SizeOfHeaders);
	DebugMsg("XeXtractor", "SecInfoOff : %08x", Header->SecurityInfo);
	DebugMsg("XeXtractor", "OptHeadCnt : %08x", Header->HeaderDirectoryEntryCount);
	DebugMsg("XeXtractor", "GameRegion : %08x", SecurityInfo->ImageInfo.GameRegion);

	return S_OK;
}
BOOL XeXtractor::IsKinectEnabled() {

	// Check 3 different privilages to get a good idea if this is a kinect game
	if(CheckExecutablePrivilege(XEX_PRIVILEGE_NATAL_TILT_CONTROL) ||
		CheckExecutablePrivilege(XEX_PRIVILEGE_TITLE_REQUIRES_SKELETAL_TRACKING) ||
		CheckExecutablePrivilege(XEX_PRIVILEGE_TITLE_SUPPORTS_SKELETAL_TRACKING))
		return TRUE;

	// None of them are enabled, so we are guessing this isnt a kinect game
	return FALSE;
}
BOOL XeXtractor::CheckExecutablePrivilege(DWORD Privilege) {

	// Get the key to use
	DWORD privilegeKey = ((Privilege & 0xFFFFFFE0) + 0x6000) << 3;

	// See if we have it in our header, if not just return false
	DWORD* privilegeValue = NULL;
	if(GetOptionalHeader(privilegeKey, (PVOID*)&privilegeValue) != S_OK)
		return FALSE;
	
	// We have it now lets check our value
	DWORD privilageMask = 1 << (Privilege & 0x1F);
	if((*privilegeValue & privilageMask) == privilageMask)
		return TRUE;

	// This privilege isnt enabled
	return FALSE;
}