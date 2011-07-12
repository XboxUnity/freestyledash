#pragma once
#include "../tools.h"

#define DDS_MAGIC                   0x20534444 // "DDS "

#define D3DFMT_FOURCC_DXT1			0x31545844 // "1TXD"
#define D3DFMT_FOURCC_DXT4			0x34545844 // "4TXD"
#define D3DFMT_FOURCC_DXT5			0x35545844 // "5TXD"

#define DDSD_CAPS                   0x00000001
#define DDSD_HEIGHT                 0x00000002
#define DDSD_WIDTH                  0x00000004
#define DDSD_PITCH                  0x00000008
#define DDSD_PIXELFORMAT            0x00001000
#define DDSD_MIPMAPCOUNT            0x00020000
#define DDSD_LINEARSIZE             0x00080000
#define DDSD_DEPTH                  0x00800000

#define DDPF_ALPHAPIXELS            0x00000001
#define DDPF_FOURCC                 0x00000004
#define DDPF_RGB                    0x00000040

#define DDSCAPS_COMPLEX             0x00000008
#define DDSCAPS_TEXTURE             0x00001000
#define DDSCAPS_MIPMAP              0x00400000

#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000

#define DDPIXELFORMAT               _DDPIXELFORMAT
#define DDSCAPS2                    _DDSCAPS2
#define DDSURFACEDESC               _DDSURFACEDESC
#define DDSHEADER                   _DDSHEADER

typedef struct _DDPIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
} DDPIXELFORMAT, *PDDPIXELFORMAT;

typedef struct _DDSCAPS2 {
	DWORD dwCaps;
	DWORD dwCaps2;
	DWORD Reserved[2];
} DDSCAPS2, *PDDSCAPS2;

typedef struct _DDSURFACEDESC {
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDPIXELFORMAT   ddpfPixelFormat;
	DDSCAPS2        ddsCaps;
	DWORD           dwReserved2;
} DDSURFACEDESC, *PDDSURFACEDESC;

typedef struct _DDSHEADER {
	DWORD           dwMagic;
	DDSURFACEDESC   ddsd;
} DDSHEADER;

extern const BYTE DxtHeader[128];

// Swap Functions
void DDSSwapHeader(DDSHEADER* Header);
void DDSSwap16(WORD* Value);
void DDSSwap32(DWORD* Value);

// DXT1 Conversion Functions
HRESULT ConvertImageFileToDXT1(string szDestFileName, string szSrcFileName);
HRESULT ConvertImageFileToDXT1Buffer(string szSrcFileName, LPBYTE * bytes, long* len);

HRESULT ConvertImageInMemoryToDXT1(string szDestFileName, BYTE * fileBuffer, DWORD fileSize);
HRESULT ConvertImageInMemoryToDXT1Buffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len);

// DXT5 Conversion Functions
HRESULT ConvertImageFileToDXT5(string szDestFileName, string szSrcFileName);
HRESULT ConvertImageFileToDXT5Buffer(string szSrcFileName, LPBYTE * bytes, long* len);

HRESULT ConvertImageInMemoryToDXT5(string szDestFileName, BYTE * fileBuffer, DWORD fileSize);
HRESULT ConvertImageInMemoryToDXT5Buffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len);

// DXTx Conversion Function
HRESULT ConvertImageFileToDXTx(string szDestFileName, string szSrcFileName, D3DFORMAT format);
HRESULT ConvertImageFileToDXTxBuffer(string szSrcFileName, LPBYTE * bytes, long* len, D3DFORMAT format);

HRESULT ConvertImageInMemoryToDXTx(string szDestFileName, BYTE * fileBuffer, DWORD fileSize, D3DFORMAT format);
HRESULT ConvertImageInMemoryToDXTxBuffer(BYTE * fileBuffer, DWORD fileSize, LPBYTE* bytes, long* len, D3DFORMAT format);