#pragma once
#include "stdafx.h"

#include "../../Application/FreestyleApp.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "../Managers/XZP/XZPManager.h"
#include "TextureItem/TextureItem.h"
#include "../Threads/Threadlock.h"
#include "TextureCache.h"

HRESULT TextureCache::LoadXZPResourceToMemory( string fileName, BYTE ** fileData, DWORD * fileSize)
{
	BYTE * fileBuffer;
	UINT fileLength;

	vector<string> m_Strings;
	StringSplit(fileName, "#", &m_Strings);

	if(m_Strings.size() != 2) {
		//DebugMsg("TextureCache", "XZP Resource Path is invalid.");
		return 1;
	}

	HRESULT retVal = NULL;
	retVal = XZPManager::getInstance().XZPOpenMemory(m_Strings.at(0), m_Strings.at(1), &fileBuffer, &fileLength);
	if(retVal != S_OK){
		//DebugMsg("TextureCache", "XZP Resource failed to extract from archive");
		return 2;
	}

	*fileData = fileBuffer;
	*fileSize = (DWORD)fileLength;

	return S_OK;
}


HRESULT TextureCache::LoadFileToMemory( string fileName, BYTE ** fileData, DWORD * fileSize )
{
	// Create Handle and Buffer Arrays 
	FILE * fHandle;
	BYTE * fileBuffer;
	DWORD fileLength;

	// Open file for reading
	fopen_s(&fHandle, fileName.c_str(), "rb");
	if(!fHandle) { 
		//DebugMsg("TextureCache", "LoadFileToMemory - Error Reading File: %s", fileName.c_str());
		return 1;
	}
	
	// Get file length 
	fseek(fHandle, 0, SEEK_END); 
	fileLength = ftell(fHandle); 
	fseek(fHandle, 0, SEEK_SET); 
		
	// Allocate memory
	fileBuffer = (BYTE *)malloc(fileLength + 1); 
	//DebugMsg("TEST", "Test-B %X", fileBuffer);
	if(!fileBuffer) { 
		//DebugMsg("TextureCache", "LoadFileToMemory - Error Allocating fileBuffer: %s", fileName.c_str());
		fclose(fHandle); 
		return 2;
	} 
	// Read file into Buffer
	fread(fileBuffer, 1, fileLength, fHandle); 
	
	// Clean up
	fclose(fHandle); 

	// Return pointer to fileBuffer
	*fileData = fileBuffer;
	*fileSize = fileLength;

	return S_OK;
}

HRESULT TextureCache::LoadD3DTextureFromFileInMemory( BYTE * fileData, DWORD fileSize, UINT nTextureWidth, UINT nTextureHeight, IDirect3DTexture9 ** pTexture, bool UseXuiLoaderOnFail)
{
	HRESULT retVal = NULL;

	retVal = D3DXCreateTextureFromFileInMemoryEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		fileData,
		fileSize,
		nTextureWidth,
		nTextureHeight,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_LIN_DXT5,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_BOX,
		D3DX_FILTER_BOX,
		0,
		NULL,
		NULL,
		pTexture
	);
	if (retVal != S_OK && retVal != D3DXERR_INVALIDDATA) {
		DebugMsg("TextureCAche", "Retval was %0X", retVal);
	}

	if(retVal == D3DXERR_INVALIDDATA && UseXuiLoaderOnFail)
	{
		// Occassionally Direct 3D will failed a texture create call due to invalid image data (xex problem?)
		XUIImageInfo imgInfo;
		retVal = XuiCreateTextureFromPNG(
			fileData, 
			fileSize,
			CFreestyleUIApp::getInstance().getXuiDevice(),
			pTexture,
			&imgInfo
		);
	}

	return retVal;

}
HRESULT TextureCache::LoadD3DTextureFromFile(string szFilePath, UINT nTextureWidth, UINT nTextureHeight, IDirect3DTexture9 ** pTexture)
{
	HRESULT retVal = NULL;

	retVal = D3DXCreateTextureFromFileExA(
		CFreestyleApp::getInstance().m_pd3dDevice,
		szFilePath.c_str(),
		nTextureWidth,
		nTextureHeight,
		1,
		0,
		D3DFMT_DXT5,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_BOX,
		D3DX_FILTER_BOX,
		0,
		NULL,
		NULL,
		pTexture
	);

	return retVal;
}