#pragma once
#include "../../Generic/xboxtools.h"

// State Definitions
#define FSDTEX_FILEINIT		0x01
#define FSDTEX_FILEREADY	0x01
#define FSDTEX_FILELOADING  0x02
#define FSDTEX_FILELOADED   0x03
#define FSDTEX_FILEERROR	0x04

#define FSDTEX_TEXINIT		0x05
#define FSDTEX_TEXREADY		0x05
#define FSDTEX_TEXLOADING	0x06
#define FSDTEX_TEXLOADED	0x07
#define FSDTEX_TEXERROR		0x08

#define FSDTEX_BRUSHINIT	0x09
#define FSDTEX_BRUSHREADY	0x09
#define FSDTEX_BRUSHLOADING 0x0A
#define FSDTEX_BRUSHLOADED  0x0B
#define FSDTEX_BRUSHERROR   0x0C

#define FSDTEX_INITIALIZED  0xF0
#define FSDTEX_LOADING		0xF1
#define FSDTEX_ALLREADY		0xF2
#define FSDTEX_INACTIVE		0xF3
#define FSDTEX_FATALERROR	0xFF

class TextureItem;

typedef vector<TextureItem *>*		TextureItemListPointer;
typedef vector<TextureItem *>		TextureItemList;

class TextureItem 
{
public:
	// Constructor / Deconstructor
	TextureItem();
	~TextureItem();

	// Set Private Variables
	HRESULT SetTextureInfo(string filePath, UINT nWidth, UINT nHeight, BOOL bXZPTexture = FALSE);
	HRESULT SetTextureInfoMemory(UINT nWidth, UINT nHeight, BOOL bRetainSource);
	HRESULT SetTextureBrush( HXUIBRUSH hBrush );
	HRESULT SetD3DTexture( IDirect3DTexture9 * pTexture );
	HRESULT SetFilePointer( BYTE * fileData, DWORD fileSize );
	HRESULT SetCurrentFileState( DWORD dwFileState );
	HRESULT SetCurrentTextureState( DWORD dwTextureState );
	HRESULT SetCurrentBrushState( DWORD dwBrushState );
	HRESULT SetTextureItemState( DWORD dwItemState );
	HRESULT SetRetainSource( BOOL bRetainSource );
	HRESULT SetXZPMode( BOOL bUseXZP );
	
	// Reference Count
	void AddRef( void ) { m_dwReferenceCount++; }
	void Release( void ) { if(GetRefCount() > 0) m_dwReferenceCount--;}
	int GetRefCount( void ) { return m_dwReferenceCount; }

	// Get Private Variables
	string GetTextureFilePath( void );
	UINT GetTextureWidth( void );
	UINT GetTextureHeight( void );
	IDirect3DTexture9 * GetD3DTexture( void );
	HXUIBRUSH GetTextureBrush( void );
	HRESULT GetFilePointer( BYTE ** fileData, DWORD * fileSize);
	DWORD GetCurrentFileState( void );
	DWORD GetCurrentTextureState( void );
	DWORD GetCurrentBrushState( void );
	DWORD GetTextureItemState( void );
	long GetPriority() { return m_priority; }
	BOOL IsXZPTexture( void ) { return m_bXZPTexture; }
	BOOL IsRetainSource( void ) { return m_bRetainSource; }
	void BumpPriority() { m_priority++; }

	// Public Methods
	HRESULT ReleaseBrush( void );
	HRESULT ReleaseTexture( void );
	HRESULT ReleaseFilePointer( void );
	HRESULT ReleaseAll( void );

private:
	// State Variables
	DWORD				m_dwCurrentFileState;
	DWORD				m_dwCurrentTextureState;
	DWORD				m_dwCurrentBrushState;
	DWORD				m_dwTextureItemState;
	
	BOOL				m_bXZPTexture;
	BOOL				m_bRetainSource;
	int					m_dwReferenceCount;
	long				m_priority;

	// Private Variables
	string				m_szFilePath;
	UINT				m_nTextureHeight;
	UINT				m_nTextureWidth;
	HXUIBRUSH			m_hTextureBrush;
	IDirect3DTexture9 *	m_pTexture;

	BYTE *				m_pFilePointer;
	DWORD				m_dwFileLength;

	// Private Methods
	HRESULT CalculateTextureItemState();



};