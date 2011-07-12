#include "stdafx.h"
#include "BMPTools.h"

#include "../../../Application/FreestyleApp.h"
#include "../../Debug/Debug.h"
#include "../xboxtools.h"

void ConvertImageFileToBMPBuffer(string szSrcFileName, LPBYTE * bytes, long* len)
{
	if(!FileExists(szSrcFileName))
		return;

	ATG::Timer m_Timer;

	double timeA = m_Timer.GetAbsoluteTime();

	IDirect3DTexture9 * pTexture;
	HRESULT retVal = D3DXCreateTextureFromFileEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		szSrcFileName.c_str(),
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_LIN_A8R8G8B8,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);


	float timeB = m_Timer.GetAbsoluteTime();
	DebugMsg("Test", "Texture Creation:  %4.2f", (timeB- timeA));

	if(retVal == S_OK) {
		timeA = m_Timer.GetAbsoluteTime();

		// Get our level desc
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		// Now lock our data
		D3DLOCKED_RECT lock;
		RECT rect = {0, 0, desc.Width, desc.Height};
		pTexture->LockRect(0, &lock, &rect, D3DLOCK_READONLY);
	
		//Read our data
		DWORD headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		DWORD dataLen = lock.Pitch * desc.Height;
		DWORD * dataBuffer = (DWORD*)malloc( dataLen + headerSize );
		DWORD * address = (DWORD*)lock.pBits;

		// Create file our header
		BITMAPFILEHEADER* fHead = (BITMAPFILEHEADER*)dataBuffer;
		fHead->bfType = 0x424D; // "BM"
		fHead->bfSize = SwapDWORD(dataLen + headerSize);
		fHead->bfReserved1 = 0;
		fHead->bfReserved2 = 0;
		fHead->bfOffBits = SwapDWORD(headerSize);

		// Create our info header
		BITMAPINFOHEADER* iHead = (BITMAPINFOHEADER*)(fHead + 1);
		ZeroMemory(iHead, sizeof(BITMAPINFOHEADER));
		iHead->biSize = SwapDWORD(sizeof(BITMAPINFOHEADER));
		iHead->biWidth = SwapDWORD(desc.Width);
		iHead->biHeight = SwapDWORD(desc.Height);
		iHead->biPlanes = SwapWORD(1);
		iHead->biBitCount = SwapWORD(32);
		iHead->biSizeImage = SwapDWORD(dataLen);

		// Copy over our raw (BGRA)
		DWORD* rawPtr = (DWORD*)(iHead + 1);
		for(int y = desc.Height - 1; y >= 0; y--) {
			for(DWORD x = 0; x < desc.Width; x++) {

				DWORD cp = (y * lock.Pitch) + (x * 4);
				DWORD * temp = (DWORD*)(address + (cp / 4));
				*rawPtr = SwapDWORD(*temp);				
				rawPtr++;
			}
		}
		
		// Unlock our texture
		pTexture->UnlockRect(0);

		timeB = m_Timer.GetAbsoluteTime();
		DebugMsg("Test", "Texture Lock Time:  %4.2f", (timeB- timeA));
		
		*len = (headerSize + dataLen);
		*bytes = (BYTE*)dataBuffer;
	}else {
		DebugMsg("ConvertImageInMemoryToBMPBuffer", 
			"Conversion To BMP From Memory Failed. [%X]", retVal);		
	}

	// Release our Texture
	if(pTexture != NULL)
		pTexture->Release();
}

HRESULT ConvertMemoryBufferToBMPBuffer( MemoryBuffer &image_in, MemoryBuffer &image_out)
{
	// Check to ensure that our data has a valid length before proceeding
	if( image_in.GetDataLength() == 0 ) return S_FALSE;

	// Set up a timer class to profile our code
	ATG::Timer m_Timer;
	double timeStart, timeStop;

	// Begin Profiling our Texture Creation Code
	timeStart = m_Timer.GetAbsoluteTime();

	IDirect3DTexture9 * pTexture;
	HRESULT retVal = D3DXCreateTextureFromFileInMemoryEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		image_in.GetData(),
		image_in.GetDataLength(),
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_LIN_A8R8G8B8,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);

	// End Profiling our Texture Creation Code
	timeStop = m_Timer.GetAbsoluteTime();
	DebugMsg("ConvertImageToBMP", "Texture Creation took %4.2f seconds to complete", (float)(timeStop - timeStart));

	// If our texture was created successfully, 
	if(retVal = D3D_OK) {

		//Begin Profiling our Data Manipulation Code
		timeStart = m_Timer.GetAbsoluteTime();

		// Get our level desc
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		// Now lock our data
		D3DLOCKED_RECT lock;
		RECT rect = {0, 0, desc.Width, desc.Height};
		pTexture->LockRect(0, &lock, &rect, D3DLOCK_READONLY);
	
		//Read our data
		DWORD headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		DWORD dataLen = lock.Pitch * desc.Height;
		DWORD * dataBuffer = (DWORD*)malloc( dataLen + headerSize );
		DWORD * address = (DWORD*)lock.pBits;

		// Create file our header
		BITMAPFILEHEADER* fHead = (BITMAPFILEHEADER*)dataBuffer;
		fHead->bfType = 0x424D; // "BM"
		fHead->bfSize = SwapDWORD(dataLen + headerSize);
		fHead->bfReserved1 = 0;
		fHead->bfReserved2 = 0;
		fHead->bfOffBits = SwapDWORD(headerSize);

		// Create our info header
		BITMAPINFOHEADER* iHead = (BITMAPINFOHEADER*)(fHead + 1);
		ZeroMemory(iHead, sizeof(BITMAPINFOHEADER));
		iHead->biSize = SwapDWORD(sizeof(BITMAPINFOHEADER));
		iHead->biWidth = SwapDWORD(desc.Width);
		iHead->biHeight = SwapDWORD(desc.Height);
		iHead->biPlanes = SwapWORD(1);
		iHead->biBitCount = SwapWORD(32);
		iHead->biSizeImage = SwapDWORD(dataLen);

		// Copy over our raw (BGRA)
		DWORD* rawPtr = (DWORD*)(iHead + 1);
		for(int y = desc.Height - 1; y >= 0; y--) {
			for(DWORD x = 0; x < desc.Width; x++) {

				DWORD cp = (y * lock.Pitch) + (x * 4);
				DWORD * temp = (DWORD*)(address + (cp / 4));
				*rawPtr = SwapDWORD(*temp);				
				rawPtr++;
			}
		}
		
		// Unlock our texture
		pTexture->UnlockRect(0);

		// End Profiling our Data Modification Code
		timeStop = m_Timer.GetAbsoluteTime();
		DebugMsg("ConvertImageToBMP", "Data Modification took %4.2f seconds to complete", (float)(timeStop - timeStart));

		// Begin Profiling our Memory Copy Code
		timeStart = m_Timer.GetAbsoluteTime();
		
		// Copy our completed data to our new buffer
		image_out.Add(dataBuffer, (headerSize + dataLen));

		// End Profiling our Memory Copy Code
		timeStop = m_Timer.GetAbsoluteTime();
		DebugMsg("ConvertImageToBMP", "Image Memory Copy took %4.2f seconds to complete", (float)(timeStop - timeStart));

	}
	else {
		DebugMsg("ConvertImageInMemoryToBMPBuffer", "Conversion To BMP From Memory Failed. [%X]", retVal);
	}

	return retVal;
}


void ConvertImageInMemoryToBMP(string szDestFileName, BYTE * fileBuffer, DWORD fileSize)
{
	// Check file size
	if (fileSize == 0) {
		DebugMsg("ConvertToBMP", "Bailing since filesize is 0");
		return;
	}

	// Convert our dds in memory
	BYTE* bmpData = NULL; long bmpSize = 0;
	ConvertImageInMemoryToBMPBuffer(fileBuffer, fileSize, &bmpData, &bmpSize);

	// Write to file if its valid
	if(bmpSize != 0) {

		FILE * fHandle;
		fopen_s(&fHandle, szDestFileName.c_str(), "wb");
		fwrite(bmpData, bmpSize, 1, fHandle);
		fclose(fHandle);

		// Free our data
		free(bmpData);
	}
}

void ConvertImageInMemoryToBMPBuffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len)
{



	// Set our initial values
	*bytes = NULL;
	*len = 0;

	// Check file size
	if (fileSize == 0) {
		DebugMsg("ConvertToPNG", "Bailing since filesize is 0");
		return;
	}

	ATG::Timer m_Timer;

	double timeA = m_Timer.GetAbsoluteTime();

	IDirect3DTexture9 * pTexture;
	HRESULT retVal = D3DXCreateTextureFromFileInMemoryEx(
		CFreestyleApp::getInstance().m_pd3dDevice,
		fileBuffer,
		fileSize,
		D3DX_DEFAULT_NONPOW2,
		D3DX_DEFAULT_NONPOW2,
		1,
		D3DUSAGE_CPU_CACHED_MEMORY,
		D3DFMT_LIN_A8R8G8B8,
		D3DPOOL_DEFAULT,
		D3DX_FILTER_NONE,
		D3DX_FILTER_NONE,
		0,
		NULL,
		NULL,
		&pTexture
	);

	float timeB = m_Timer.GetAbsoluteTime();
	DebugMsg("Test", "Texture Creation:  %4.2f", (timeB- timeA));

	if(retVal == S_OK) {
		timeA = m_Timer.GetAbsoluteTime();

		// Get our level desc
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		// Now lock our data
		D3DLOCKED_RECT lock;
		RECT rect = {0, 0, desc.Width, desc.Height};
		pTexture->LockRect(0, &lock, &rect, D3DLOCK_READONLY);
	
		//Read our data
		DWORD headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		DWORD dataLen = lock.Pitch * desc.Height;
		DWORD * dataBuffer = (DWORD*)malloc( dataLen + headerSize );
		DWORD * address = (DWORD*)lock.pBits;

		// Create file our header
		BITMAPFILEHEADER* fHead = (BITMAPFILEHEADER*)dataBuffer;
		fHead->bfType = 0x424D; // "BM"
		fHead->bfSize = SwapDWORD(dataLen + headerSize);
		fHead->bfReserved1 = 0;
		fHead->bfReserved2 = 0;
		fHead->bfOffBits = SwapDWORD(headerSize);

		// Create our info header
		BITMAPINFOHEADER* iHead = (BITMAPINFOHEADER*)(fHead + 1);
		ZeroMemory(iHead, sizeof(BITMAPINFOHEADER));
		iHead->biSize = SwapDWORD(sizeof(BITMAPINFOHEADER));
		iHead->biWidth = SwapDWORD(desc.Width);
		iHead->biHeight = SwapDWORD(desc.Height);
		iHead->biPlanes = SwapWORD(1);
		iHead->biBitCount = SwapWORD(32);
		iHead->biSizeImage = SwapDWORD(dataLen);

		// Copy over our raw (BGRA)
		DWORD* rawPtr = (DWORD*)(iHead + 1);
		for(int y = desc.Height - 1; y >= 0; y--) {
			for(DWORD x = 0; x < desc.Width; x++) {

				DWORD cp = (y * lock.Pitch) + (x * 4);
				DWORD * temp = (DWORD*)(address + (cp / 4));
				*rawPtr = SwapDWORD(*temp);				
				rawPtr++;
			}
		}
		
		// Unlock our texture
		pTexture->UnlockRect(0);

		timeB = m_Timer.GetAbsoluteTime();
		DebugMsg("Test", "Texture Lock Time:  %4.2f", (timeB- timeA));
		
		*len = (headerSize + dataLen);
		*bytes = (BYTE*)dataBuffer;
	}else {
		DebugMsg("ConvertImageInMemoryToBMPBuffer", 
			"Conversion To BMP From Memory Failed. [%X]", retVal);		
	}

	// Release our Texture
	if(pTexture != NULL)
		pTexture->Release();
}