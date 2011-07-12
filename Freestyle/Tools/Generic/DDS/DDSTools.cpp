#pragma once
#include "stdafx.h"
#include "DDSTools.h"

#include "../../../Application/FreestyleApp.h"
#include "../../Debug/Debug.h"

const BYTE DxtHeader[128] = { 
	0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x00, 
	0x00, 0xD0, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x20, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x02, 0x10, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};


void DDSSwapHeader(DDSHEADER* Header) {
	
	// Just loop through since its all dwords anyways :)
	int size = sizeof(DDSHEADER) / sizeof(DWORD);
	for(int x = 0; x < size; x++)
		DDSSwap32(&((DWORD*)Header)[x]);
}

void DDSSwap32(DWORD* Value){
	*Value = (((*Value & 0xFF000000) >> 24) | 
		((*Value & 0xFF0000) >> 8) | 
		((*Value & 0xFF00) << 8) | 
		((*Value & 0xFF) << 24));
}

void DDSSwap16(WORD* Value) {
	*Value = (((*Value & 0xFF00) >> 8) |
		((*Value & 0xFF) << 8));
}

// DXT1 Conversion Functions
HRESULT ConvertImageFileToDXT1( string szDestFileName, string szSrcFileName ) {
	return ConvertImageFileToDXTx( szDestFileName, szSrcFileName, D3DFMT_LIN_DXT1 );
}

HRESULT ConvertImageFileToDXT1Buffer( string szSrcFileName, LPBYTE * bytes, long *len) {
	return ConvertImageFileToDXTxBuffer( szSrcFileName, bytes, len, D3DFMT_LIN_DXT1 );
}

HRESULT ConvertImageInMemoryToDXT1( string szDestFileName, BYTE* fileBuffer, DWORD fileSize ) {
	return ConvertImageInMemoryToDXTx( szDestFileName, fileBuffer, fileSize, D3DFMT_LIN_DXT1 );
}

HRESULT ConvertImageInMemoryToDXT1Buffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len) {
	return ConvertImageInMemoryToDXTxBuffer(fileBuffer, fileSize, bytes, len, D3DFMT_LIN_DXT1);
}

// DXT5 Conversion Functions
HRESULT ConvertImageFileToDXT5( string szDestFileName, string szSrcFileName ) {
	return ConvertImageFileToDXTx( szDestFileName, szSrcFileName, D3DFMT_LIN_DXT5 );
}

HRESULT ConvertImageFileToDXT5Buffer( string szSrcFileName, LPBYTE * bytes, long *len) {
	return ConvertImageFileToDXTxBuffer( szSrcFileName, bytes, len, D3DFMT_LIN_DXT5 );
}

HRESULT ConvertImageInMemoryToDXT5( string szDestFileName, BYTE* fileBuffer, DWORD fileSize ) {
	return ConvertImageInMemoryToDXTx( szDestFileName, fileBuffer, fileSize, D3DFMT_LIN_DXT5 );
}

HRESULT ConvertImageInMemoryToDXT5Buffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len) {
	return ConvertImageInMemoryToDXTxBuffer(fileBuffer, fileSize, bytes, len, D3DFMT_LIN_DXT5);
}


// Generalized DXT Converstion Functions

HRESULT ConvertImageFileToDXTx(string szDestFileName, string szSrcFileName, D3DFORMAT format) {
	
	HRESULT retVal = NULL;
	
	// Open the file for reading
	FILE * fSrcFile;
	fopen_s(&fSrcFile, szSrcFileName.c_str(), "rb");

	// Determine size of file
	fseek(fSrcFile, 0, SEEK_END);
	long fileSize = ftell(fSrcFile);
	fseek(fSrcFile, 0, SEEK_SET);

	if(fileSize == 0) {
		DebugMsg("DDSTools", "[ConvertImageFileToDXTx] Invalid Data found, File Size = 0");
		fclose(fSrcFile);
		return S_FALSE;
	}

	// Allocate a buffer to hold file data
	LPBYTE fileData = (LPBYTE)malloc(fileSize);
	
	// Read the file data into the buffer and then close the file handle
	fread(fileData, fileSize, 1, fSrcFile);
	fclose(fSrcFile);

	retVal = ConvertImageInMemoryToDXTx(szDestFileName, fileData, fileSize, format);
	if(fileData != NULL)
		free(fileData);

	return retVal;
}

HRESULT ConvertImageFileToDXTxBuffer(string szSrcFileName, LPBYTE *bytes, long *len, D3DFORMAT format) {
	
	HRESULT retVal = NULL;
	
	// Open the file for reading
	FILE * fSrcFile;
	fopen_s(&fSrcFile, szSrcFileName.c_str(), "rb");

	// Determine size of file
	fseek(fSrcFile, 0, SEEK_END);
	long fileSize = ftell(fSrcFile);
	fseek(fSrcFile, 0, SEEK_SET);

	if(fileSize == 0) {
		DebugMsg("DDSTools", "[ConvertImageFileToDXTxBuffer] Invalid Data found, File Size = 0");
		fclose(fSrcFile);
		return S_FALSE;
	}

	// Allocate a buffer to hold file data
	LPBYTE fileData = (LPBYTE)malloc(fileSize);
	
	// Read the file data into the buffer and then close the file handle
	fread(fileData, fileSize, 1, fSrcFile);
	fclose(fSrcFile);

	retVal = ConvertImageInMemoryToDXTxBuffer(fileData, fileSize, bytes, len, format);
	if(fileData != NULL)
		free(fileData);

	return retVal;
}

HRESULT ConvertImageInMemoryToDXTx(string szDestFileName, BYTE * fileBuffer, DWORD fileSize, D3DFORMAT format) {
	
	HRESULT retVal = NULL;

	// Check file size
	if (fileSize == 0) {
		DebugMsg("DDSTools", "[ConvertImageInMemoryToDXTx] Invalid Data found, File Size = 0");
		return S_FALSE;
	}

	// Convert our dds in memory
	BYTE* ddsData = NULL; long ddsSize = 0;
	retVal = ConvertImageInMemoryToDXTxBuffer(fileBuffer, fileSize, &ddsData, &ddsSize, format);

	// Write to file if its valid
	if(retVal == S_OK) {

		FILE * fHandle;
		fopen_s(&fHandle, szDestFileName.c_str(), "wb");
		if(fHandle != NULL) {
			fwrite(ddsData, ddsSize, 1, fHandle);
			fclose(fHandle);
		}

		// Free our data
		free(ddsData);
	}

	return retVal;
}

HRESULT ConvertImageInMemoryToDXTxBuffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len, D3DFORMAT format) {
	
	// Set our initial values
	*bytes = NULL;
	*len = 0;

	// Check file size
	if (fileSize == 0) {
		DebugMsg("DDSTools", "[ConvertImageInMemoryToDXTxBuffer] Invalid Data found, File Size = 0");
		return S_FALSE;
	}

	// Handle the format type - return false if not yet supported
	int nPitchMultiplier;
	DWORD dwFourCC;
	switch (format) {
		case D3DFMT_LIN_DXT1:
			nPitchMultiplier = 2;
			dwFourCC = D3DFMT_FOURCC_DXT1;
			break;
		case D3DFMT_LIN_DXT5:
			nPitchMultiplier = 4;
			dwFourCC = D3DFMT_FOURCC_DXT5;
			break;
		default:
			return S_FALSE;
	};

	// Load File To Direct3D DXTx Texture w/ 1 mipmap
	IDirect3DTexture9 * pTexture;
	HRESULT retVal = D3DXCreateTextureFromFileInMemoryEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		fileBuffer,
		fileSize,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		format,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);

	if(retVal == D3DXERR_INVALIDDATA) {

		// Occassionally Direct 3D will failed a texture create call due to invalid image data (xex problem?)
		XUIImageInfo imgInfo;
		retVal = XuiCreateTextureFromPNG(
			fileBuffer, 
			fileSize,
			CFreestyleUIApp::getInstance().getXuiDevice(),
			&pTexture,
			&imgInfo
		);
	}

	//Save texture to dds file in memory
	if( retVal == S_OK ){ 

		// Get our level desc
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		// Now lock our data
		D3DLOCKED_RECT lock;
		RECT rect = {0, 0, desc.Width, desc.Height};
		pTexture->LockRect(0, &lock, &rect, D3DLOCK_READONLY);
		DWORD realPitch = desc.Width * nPitchMultiplier;
		
		// Create Header Struct
		DDSHEADER dds; ZeroMemory(&dds, sizeof(DDSHEADER));
		dds.dwMagic = DDS_MAGIC;
		dds.ddsd.dwSize = sizeof(DDSURFACEDESC);
		dds.ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		dds.ddsd.dwHeight = desc.Height;
		dds.ddsd.dwWidth = desc.Width;
		dds.ddsd.dwPitchOrLinearSize = realPitch * desc.Height;
		dds.ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		dds.ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
		dds.ddsd.ddpfPixelFormat.dwFourCC = dwFourCC;
		dds.ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
		DDSSwapHeader(&dds);

		// Now allocate our data
		*len = (realPitch * desc.Height) + sizeof(DDSHEADER);
		*bytes = (BYTE*)malloc(*len);

		// Copy our dds header
		memcpy(*bytes, &dds, sizeof(DDSHEADER));

		// Copy our data and swap
		WORD* data = (WORD*)(*bytes + sizeof(DDSHEADER));
		if(desc.Format == D3DFMT_LIN_A8R8G8B8)
			XGCompressSurface(data, realPitch, desc.Width, desc.Height, format, NULL, lock.pBits,
					lock.Pitch, desc.Format, NULL, 0, 0);
		else
			XGCopySurface(data, realPitch, desc.Width, desc.Height, format, NULL, lock.pBits,
					lock.Pitch, desc.Format, NULL, 0, 0);

		XGEndianSwapSurface(data, realPitch, data, realPitch, desc.Width, desc.Height, format);

		// Unlock
		pTexture->UnlockRect(0);
	}
	else {
		DebugMsg("DDSTools", "Conversion To DXT From Memory Failed. [%X]", retVal);	
	}

	//Release Texture
	if(pTexture != NULL)
		pTexture->Release();
	pTexture = NULL;

	return retVal;
}