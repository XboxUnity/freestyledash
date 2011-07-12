#pragma once
#include "stdafx.h"

#include "./QueueThreads/FileQueue.h"
#include "./QueueThreads/FileTexQueue.h"
#include "./QueueThreads/TextureQueue.h"
#include "./QueueThreads/ReleaseQueue.h"
#include "./WorkerThreads/FileWorker.h"
#include "./WorkerThreads/FileTexWorker.h"
#include "./WorkerThreads/TextureWorker.h"
#include "./WorkerThreads/ReleaseWorker.h"

#include "../Managers/Skin/SkinManager.h"

#include "../Managers/GameList/GameListManager.h"
#include "GameContentManager.h"

HRESULT GameContentManager::ManageContentTextures( void )
{
	
	// Loadup Screenshots if User has this setting activated
	if(m_bDisplayScreenshots)
		HandleScreenshotTextures();		
	
	// Loadup Background if User has this setting activated
	if(m_bDisplayBackground)
		HandleBackgroundTexture();

	return S_OK;
}

HRESULT GameContentManager::HandleScreenshotTextures( void )
{

	GAMECONTENT_LIST_STATE listPack = m_sListState;

	// If the PreviousWGAME Pointer is not NULL
	if(listPack.PreviousGame !=NULL)
	{
		// And the Previous SS1Texture is not NULL
		if(listPack.PreviousGame->Textures.m_SS1Texture != NULL)
		{
			// Then decrease the reference count
			listPack.PreviousGame->Textures.m_SS1Texture->Release();
			// Add the texture to the Release Queue for a full release
			ReleaseQueue::getInstance().AddToQueue(listPack.PreviousGame->Textures.m_SS1Texture, true, true, true);
		}

		// And the Previous SS2Texture is not NULL
		if(listPack.PreviousGame->Textures.m_SS2Texture != NULL)
		{
			// Then decrease the reference count
			listPack.PreviousGame->Textures.m_SS2Texture->Release();
			// Add the texture to the Release Queue for a full release
			ReleaseQueue::getInstance().AddToQueue(listPack.PreviousGame->Textures.m_SS2Texture, true, true, true);
		}
	}

	// If the CurrentWGAME Pointer is not NULL
	if(listPack.CurrentGame !=NULL)
	{
		// And the Screenshot 1 Texture is not NULL
		if(listPack.CurrentGame->Textures.m_SS1Texture != NULL)
		{
			// Initialize the texture with a path and texture size
			if(listPack.CurrentGame->ContentRef->GetAssetFlags() & CONTENT_ASSET_SCREENSHOT) {
				string szScreenshot1Path = listPack.CurrentGame->ContentRef->getScreenshotPath(0);
				listPack.CurrentGame->Textures.m_SS1Texture->SetTextureInfo(szScreenshot1Path, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);
				// Increase TextureItem's reference count by one
				listPack.CurrentGame->Textures.m_SS1Texture->AddRef();

				// Add to FileTex Queue for processing
				FileTexQueue::getInstance().AddToQueue(listPack.CurrentGame->Textures.m_SS1Texture);
			}
		}

		// And Screenshot 2 TextureItem is not NULL
		if(listPack.CurrentGame->Textures.m_SS2Texture != NULL)
		{
			if(listPack.CurrentGame->ContentRef->GetAssetFlags() & CONTENT_ASSET_SCREENSHOT) {
				// Initialize the texture with a path and texture size
				string szScreenshot2Path = listPack.CurrentGame->ContentRef->getScreenshotPath(1);
				listPack.CurrentGame->Textures.m_SS2Texture->SetTextureInfo(szScreenshot2Path, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2);
				// Increase TextureItem's reference count by one
				listPack.CurrentGame->Textures.m_SS2Texture->AddRef();

				// Add to FileTex Queue for Processing
				FileTexQueue::getInstance().AddToQueue(listPack.CurrentGame->Textures.m_SS2Texture);
			}
		}
	}
	// Return Successfully
	return S_OK;
}

HRESULT GameContentManager::HandleBackgroundTexture( void )
{
	// Store a snapshot of the list state for upcoming texture loading
	GAMECONTENT_LIST_STATE listPack = m_sListState;

	// If the CurrentWGAME Pointer is not NULL
	if(listPack.CurrentGame !=NULL)
	{
		// And the Background TextureItem Pointer is not NULL
		if(listPack.CurrentGame->Textures.m_BkgTexture != NULL)
		{
			if(listPack.CurrentGame->ContentRef->GetAssetFlags() & CONTENT_ASSET_BACKGROUND) {
				// Initialize the texture with a path and texture size
				string szBackgroundPath = listPack.CurrentGame->ContentRef->getBackgroundPath();
				listPack.CurrentGame->Textures.m_BkgTexture->SetTextureInfo(szBackgroundPath, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2); 
				// Increase the TextureItem's reference count
				listPack.CurrentGame->Textures.m_BkgTexture->AddRef();
				// Add the TextureItem to the FileTexQueue For Processing
				FileTexQueue::getInstance().AddToQueue(listPack.CurrentGame->Textures.m_BkgTexture);
			}
		}
	}

	// If the PreviousWGAME is not NULL
	if(listPack.PreviousGame !=NULL)
	{
		// And the Previous BackgroundTexture is not NULL
		if(listPack.PreviousGame->Textures.m_BkgTexture != NULL)
		{
			// Then decrease its reference count
			listPack.PreviousGame->Textures.m_BkgTexture->Release();
			// Add the texture to the Release Queue for a full release
			ReleaseQueue::getInstance().AddToQueue(listPack.PreviousGame->Textures.m_BkgTexture, true, true, true);
		}
	}

	// Return Successfully
	return S_OK;
}

HXUIBRUSH GameContentManager::getSourceImageTexture(int nImageType, int * nImageIndex, GameListContentPtr listContent)
{
	if(listContent == NULL)
		return NULL;

	HXUIBRUSH retBrush = NULL;
	int nIndex = 0;

	if(nImageIndex == NULL)
		nIndex = m_sListState.CurrentIndex;
	else
		nIndex = *nImageIndex;

	bool bSpecialCase = false;
	if(m_sListState.ListSize == 0 && nImageIndex == NULL)
		bSpecialCase = true;

	if(nIndex < (int)listContent->size() && nIndex != 0xffffffff || bSpecialCase == true)
	{
		// Obtain GameListItem bject from GameListManager for Current List Index
		GameListItem * pContent = NULL;
		if(bSpecialCase == false)
			pContent = listContent->at(nIndex);

		// Extract Texture Brush from the embedded ContentItem
		TextureItem * displayTexture;
		HXUIBRUSH missingBrush;
		HXUIBRUSH loadingBrush;

		switch (nImageType)
		{
		case FSDIMAGE_GAMEICON:
			displayTexture = bSpecialCase ? NULL : pContent->Textures.m_IconTexture;
			missingBrush = getMissingIconTexture();
			loadingBrush = getLoadingIconTexture();
			break;
		case FSDIMAGE_GAMEBOXART:
			displayTexture = bSpecialCase ? NULL : pContent->Textures.m_BoxartTexture;
			missingBrush = getMissingBoxArtTexture();
			loadingBrush = getLoadingBoxArtTexture();
			break;
		case FSDIMAGE_GAMEBANNER:
			displayTexture = bSpecialCase ? NULL : pContent->Textures.m_BannerTexture;
			missingBrush = getMissingBannerTexture();
			loadingBrush = getLoadingBannerTexture();
			break;
		case FSDIMAGE_GAMEBACKGROUND:
			displayTexture = bSpecialCase ? NULL : pContent->Textures.m_BkgTexture;
			missingBrush = NULL;
			loadingBrush = NULL; 
			break;
		case FSDIMAGE_GAMESCREENSHOT1:
			displayTexture = bSpecialCase ? NULL : pContent->Textures.m_SS1Texture;
			missingBrush = getMissingScreenshotTexture();
			loadingBrush = getLoadingScreenshotTexture();
			break;
		case FSDIMAGE_GAMESCREENSHOT2:
			displayTexture = bSpecialCase ? NULL : pContent->Textures.m_SS2Texture;
			missingBrush = getMissingScreenshotTexture();
			loadingBrush = getLoadingScreenshotTexture();
			break;
		case FSDIMAGE_GAMEFAVORITE:
			displayTexture = bSpecialCase ? NULL : pContent->Flags.m_bFavorite == true ? &m_sFavoritesStar : NULL;
			missingBrush = NULL;
			loadingBrush = NULL;
			break;
		case FSDIMAGE_GAMERATINGSTAR1:
			displayTexture = bSpecialCase ? NULL : m_vUserRatingTextures.at(pContent->Content.m_vUserRating.at(0));
			missingBrush = NULL;
			loadingBrush = NULL;
			break;
		case FSDIMAGE_GAMERATINGSTAR2:
			displayTexture = bSpecialCase ? NULL : m_vUserRatingTextures.at(pContent->Content.m_vUserRating.at(1));
			missingBrush = NULL;
			loadingBrush = NULL;
			break;
		case FSDIMAGE_GAMERATINGSTAR3:
			displayTexture = bSpecialCase ? NULL : m_vUserRatingTextures.at(pContent->Content.m_vUserRating.at(2));
			missingBrush = NULL;
			loadingBrush = NULL;
			break;
		case FSDIMAGE_GAMERATINGSTAR4:
			displayTexture = bSpecialCase ? NULL : m_vUserRatingTextures.at(pContent->Content.m_vUserRating.at(3));
			missingBrush = NULL;
			loadingBrush = NULL;
			break;
		case FSDIMAGE_GAMERATINGSTAR5:
			displayTexture = bSpecialCase ? NULL : m_vUserRatingTextures.at(pContent->Content.m_vUserRating.at(4));
			missingBrush = NULL;
			loadingBrush = NULL;
			break;
		default:
			displayTexture = NULL;
			missingBrush = NULL;
			loadingBrush = NULL;
		};

		if(displayTexture != NULL && !bSpecialCase)
		{
			if(displayTexture->GetCurrentBrushState() == FSDTEX_BRUSHLOADED)
				retBrush = displayTexture->GetTextureBrush();
			else if(displayTexture->GetCurrentBrushState() == FSDTEX_BRUSHLOADING) {
				displayTexture->BumpPriority();
				retBrush = loadingBrush;
			}
			else
				retBrush = missingBrush;
		}
		else if( displayTexture == NULL && bSpecialCase) {
			retBrush = missingBrush;
		}

	}
	return retBrush;
}

HRESULT GameContentManager::LoadStaticTextures()
{
	if(m_bAttachedList == true)
	{	
		BOOL usingXZP = SkinManager::getInstance().isCurrentCompressed() == true ? 1 : 0;

		m_sLoadingBoxArt.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.LoadingBoxartPath,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		m_sLoadingIcon.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.LoadingIconPath,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		m_sLoadingBanner.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.LoadingBannerPath,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		m_sLoadingScreenshot.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.LoadingScreenshotPath,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);

		m_sMissingBoxArt.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.MissingBoxartPath, 
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		m_sMissingIcon.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.MissingIconPath, 
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		m_sMissingBanner.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.MissingBannerPath,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		m_sMissingScreenshot.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.MissingScreenshotPath, 
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);

		m_sFavoritesStar.SetTextureInfo(SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.StarFavoritePath,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
		
		for(int i = 0; i < FSDRATING_MAXSTARCOUNT; i++)
		{
			TextureItem * tex = new TextureItem();
			string filePath = SkinManager::getInstance().getCurrentScenePath() + m_sListSettings.StarRatingPath[i];
			tex->SetTextureInfo(filePath, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, usingXZP);
			m_vUserRatingTextures.push_back(tex);
		}

		return S_OK;
	}

	return S_FALSE;
}

HRESULT GameContentManager::clearGameListAssets( void )
{
	// Extract the list size from the current liststate
	int nListSize = m_sListState.ListSize;

	// Loop through each item in the list state
	for(int i = 0; i < nListSize; i++)
	{
		// Grab the pointer for the Game at index 'i'
		GameListItem * pContent = &GameListManager::getInstance().GetGameAt(m_sTabId, i);
		if(pContent->Textures.m_BkgTexture->GetRefCount() > 0)
		{
			// Release Background Texture if it has a Reference Count > 0
			pContent->Textures.m_BkgTexture->Release();
			ReleaseQueue::getInstance().AddToQueue(pContent->Textures.m_BkgTexture, true, true, true);
		}
		if(pContent->Textures.m_SS1Texture->GetRefCount() > 0)
		{
			// Release Screenshot 1 Texture if it has a Reference Count > 0
			pContent->Textures.m_SS1Texture->Release();
			ReleaseQueue::getInstance().AddToQueue(pContent->Textures.m_SS1Texture, true, true, true);
		}
		if(pContent->Textures.m_SS2Texture->GetRefCount() > 0)
		{
			// Release Screenshot 2 Texture if it has a Reference Count > 0
			pContent->Textures.m_SS2Texture->Release();
			ReleaseQueue::getInstance().AddToQueue(pContent->Textures.m_SS2Texture, true, true, true);
		}
	}

	return S_OK;
}
