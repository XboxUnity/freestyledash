#pragma once
#include "../tools.h"
#include "../../HTTP/Base/MemoryBuffer.h"

#pragma pack(1)
typedef struct _BITMAPFILEHEADER {   // bmfh
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER {    // bmih
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack()


// BMP Conversion Functions
HRESULT ConvertMemoryBufferToBMPBuffer( MemoryBuffer &image_in, MemoryBuffer &image_out);
void ConvertImageInMemoryToBMPBuffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len);
void ConvertImageInMemoryToBMP(string szDestFileName, BYTE * fileBuffer, DWORD fileSize);
void ConvertImageFileToBMPBuffer(string szSrcFileName, LPBYTE * bytes, long *len);