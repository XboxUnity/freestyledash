#pragma once
#include <map>

#include "../Threads/ThreadLock.h"
#include "TextureItem/TextureItem.h"

class TextureCache
{
public:
	// File Management Functions
	static HRESULT LoadFileToMemory( string fileName, BYTE ** fileData, DWORD * fileSize );
	static HRESULT LoadXZPResourceToMemory( string fileName, BYTE ** fileData, DWORD * fileSize);

	// Texture Management Functions
	static HRESULT LoadD3DTextureFromFileInMemory( BYTE * fileData, DWORD fileSize, UINT nTextureWidth, UINT nTextureHeight, IDirect3DTexture9 ** pTexture, bool UseXuiLoaderOnFail = true);
	static HRESULT LoadD3DTextureFromFile(string szFilePath, UINT nTextureWidth, UINT nTextureHeight, IDirect3DTexture9 ** pTexture);

	static TextureItem * GetTextureFromCache(string szTexturePath);
	static void AddTextureToCache(TextureItem * texture);
	static void RemoveTextureFromCache(string szTexturePath);

private:
	map<string, TextureItem*> m_TextureCache;
};