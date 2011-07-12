#include "stdafx.h"
#include "../../Debug/Debug.h"
#include "TextureItem.h"
#include "../../GameContent/QueueThreads/FileTexQueue.h"

TextureItem::TextureItem() 
{
	m_dwCurrentFileState = FSDTEX_INACTIVE;
	m_dwCurrentTextureState = FSDTEX_INACTIVE;
	m_dwCurrentBrushState = FSDTEX_LOADING;
	m_dwTextureItemState = FSDTEX_INACTIVE;
	
	// Initialize Variables
	if(m_pTexture != NULL)
		m_pTexture = NULL;

	m_nTextureWidth = 0;
	m_nTextureHeight = 0;
	m_szFilePath = "";
	m_hTextureBrush = NULL;
	m_pFilePointer = NULL;
	m_dwFileLength = 0;
	m_dwReferenceCount = 0;
	m_bXZPTexture = FALSE;
	m_bRetainSource = FALSE;
	m_priority = 0;
}

TextureItem::~TextureItem() {
	// Safely release stored textures and pointers
	if(m_pTexture != NULL)
		m_pTexture->Release();

	m_pTexture = NULL;
	if(m_hTextureBrush != NULL)
		XuiDestroyBrush(m_hTextureBrush);

	m_hTextureBrush = NULL;
	if(m_bRetainSource == FALSE)
	{
		if(m_pFilePointer != NULL)
			free(m_pFilePointer);
	}

	m_pFilePointer = NULL;
}


HRESULT TextureItem::ReleaseAll( void ){

	ReleaseBrush();
	ReleaseTexture();
	ReleaseFilePointer();

	return S_OK;
}

HRESULT TextureItem::ReleaseBrush( void )
{
	if(m_hTextureBrush != NULL && m_dwReferenceCount == 0) {
		FileTexQueue::getInstance().Lock();
		if(m_hTextureBrush != NULL && m_dwReferenceCount == 0) {
//			printf("%08x: TextureItem::ReleaseBrush %08x is releasing %08x\n", GetCurrentThreadId(), this, m_hTextureBrush);
			XuiDestroyBrush(m_hTextureBrush);
			m_hTextureBrush = NULL;
		}
		FileTexQueue::getInstance().Unlock();
	}

	m_hTextureBrush = NULL;
	m_dwCurrentBrushState = FSDTEX_BRUSHREADY;

	return S_OK;
}

HRESULT TextureItem::ReleaseFilePointer( void )
{
	if(m_bRetainSource == FALSE && m_pFilePointer != NULL)
	{
		FileTexQueue::getInstance().Lock();
		if(m_bRetainSource == FALSE && m_pFilePointer != NULL) {
//			printf("%08x: TextItem::ReleaseFilePointer %08x is freeing %08x\n", GetCurrentThreadId(), this, m_pFilePointer);
			free(m_pFilePointer);
			m_pFilePointer = NULL;
		}
		FileTexQueue::getInstance().Unlock();
	}

	m_pFilePointer = NULL;
	m_dwFileLength = 0;
	//DebugMsg("TextureItem", "Release File:  %s", this->m_szFilePath.c_str());
	m_dwCurrentFileState = FSDTEX_FILEREADY;
	//CalculateTextureItemState();

	return S_OK;
}

HRESULT TextureItem::ReleaseTexture( void ) {
	if(m_pTexture != NULL && m_dwReferenceCount == 0){
		FileTexQueue::getInstance().Lock();
		if(m_pTexture != NULL && m_dwReferenceCount == 0) {
			m_pTexture->Release();
			m_pTexture = NULL;
		}
		FileTexQueue::getInstance().Unlock();
	}

	m_pTexture = NULL;
	//DebugMsg("TextureItem", "Release Texture:  %s", this->m_szFilePath.c_str());
	m_dwCurrentTextureState = FSDTEX_TEXREADY;
	//CalculateTextureItemState();

	return S_OK;
}

HRESULT TextureItem::SetTextureInfo(string filePath, UINT nWidth, UINT nHeight, BOOL bXZPTexture) {
	// Set Initial Texture Information
	m_szFilePath = filePath;
	m_nTextureWidth = nWidth;
	m_nTextureHeight = nHeight;
	m_bXZPTexture = bXZPTexture;

	m_dwCurrentFileState = FSDTEX_FILEREADY;
	m_dwCurrentTextureState = FSDTEX_TEXREADY;
	m_dwCurrentBrushState = FSDTEX_LOADING;
	m_dwTextureItemState = FSDTEX_INITIALIZED;

	//AddRef();
	return S_OK;
}

HRESULT TextureItem::SetTextureInfoMemory(UINT nWidth, UINT nHeight, BOOL bRetainSource)
{
	//Set Initial Texture Information
	m_szFilePath = "";
	m_nTextureWidth = nWidth;
	m_nTextureHeight = nHeight;
	m_bXZPTexture = FALSE;
	m_bRetainSource = bRetainSource;

	m_dwCurrentFileState = FSDTEX_FILEREADY;
	m_dwCurrentTextureState = FSDTEX_TEXREADY;
	m_dwCurrentBrushState = FSDTEX_BRUSHREADY;
	m_dwTextureItemState = FSDTEX_INITIALIZED;

	return S_OK;
}

HRESULT TextureItem::CalculateTextureItemState()
{
	if(m_dwTextureItemState == FSDTEX_INITIALIZED)
	{
		if(m_dwCurrentBrushState == FSDTEX_BRUSHLOADED && m_dwCurrentTextureState == FSDTEX_TEXLOADED && m_dwCurrentFileState == FSDTEX_FILELOADED)
			m_dwTextureItemState = FSDTEX_ALLREADY;
		else
			m_dwTextureItemState = FSDTEX_INITIALIZED;
	}
	else
	{
		m_dwTextureItemState = FSDTEX_INACTIVE;
	}

	return S_OK;
}

HRESULT TextureItem::SetXZPMode( BOOL bUseXZP )
{
	m_bXZPTexture = bUseXZP;
	return S_OK;
}

HRESULT TextureItem::SetRetainSource( BOOL bRetainSource )
{
	m_bRetainSource = bRetainSource;
	return S_OK;
}

HRESULT TextureItem::SetTextureBrush( HXUIBRUSH hBrush ) {
	m_hTextureBrush = hBrush;
	
	if(hBrush != NULL)
		m_dwCurrentBrushState = FSDTEX_BRUSHLOADED;
	else
		m_dwCurrentBrushState = m_dwTextureItemState == FSDTEX_INITIALIZED ? FSDTEX_BRUSHREADY : FSDTEX_INACTIVE;

	//CalculateTextureItemState();

	return S_OK;
}

HRESULT TextureItem::SetD3DTexture( IDirect3DTexture9 * pTexture ) {
	if(pTexture != NULL)
		m_pTexture = pTexture;
	else
		m_pTexture = NULL;

	if(pTexture != NULL)
		m_dwCurrentTextureState = FSDTEX_TEXLOADED;
	else
		m_dwCurrentTextureState = m_dwTextureItemState == FSDTEX_INITIALIZED ? FSDTEX_TEXREADY : FSDTEX_INACTIVE;

	//CalculateTextureItemState();

	return S_OK;
}

HRESULT TextureItem::SetFilePointer( BYTE * fileData, DWORD fileSize ) {
	m_pFilePointer = fileData;
	m_dwFileLength = fileSize;

	if(m_pFilePointer != NULL && m_dwFileLength > 0)
		m_dwCurrentFileState = FSDTEX_FILELOADED;
	else
		m_dwCurrentFileState = m_dwTextureItemState == FSDTEX_INITIALIZED ? FSDTEX_FILEREADY : FSDTEX_INACTIVE;

	//CalculateTextureItemState();

	return S_OK;
}

HRESULT TextureItem::GetFilePointer( BYTE ** fileData, DWORD * fileSize) {
	(*fileData) = m_pFilePointer;
	(*fileSize) = m_dwFileLength;
	return S_OK;
}

HRESULT TextureItem::SetCurrentFileState( DWORD dwFileState ) {
	m_dwCurrentFileState = dwFileState;
	
	//CalculateTextureItemState();
	return S_OK;
}

HRESULT TextureItem::SetCurrentTextureState( DWORD dwTextureState ) {
	m_dwCurrentTextureState = dwTextureState;
	//CalculateTextureItemState();
	return S_OK;
}

HRESULT TextureItem::SetCurrentBrushState( DWORD dwBrushState ) {
	m_dwCurrentBrushState = dwBrushState;
	//CalculateTextureItemState();
	return S_OK;
}

HRESULT TextureItem::SetTextureItemState( DWORD dwItemState ) {
	m_dwTextureItemState = dwItemState;
	return S_OK;
}

string TextureItem::GetTextureFilePath( void ) {
	return m_szFilePath;
}

UINT TextureItem::GetTextureWidth( void ) {
	return m_nTextureWidth;
}

UINT TextureItem::GetTextureHeight( void ) {
	return m_nTextureHeight;
}

IDirect3DTexture9 * TextureItem::GetD3DTexture( void ) {
	return m_pTexture;
}

HXUIBRUSH TextureItem::GetTextureBrush( void ) {
	return m_hTextureBrush;
}

DWORD TextureItem::GetCurrentFileState( void ) {
	return m_dwCurrentFileState;
}

DWORD TextureItem::GetCurrentTextureState( void ) {
	return m_dwCurrentTextureState;
}

DWORD TextureItem::GetCurrentBrushState( void ) {
	return m_dwCurrentBrushState;
}

DWORD TextureItem::GetTextureItemState( void ) {
	return m_dwTextureItemState;
}