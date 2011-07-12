#include "stdafx.h"
#include "GameListItem.h"

GameListItem::~GameListItem()
{

	//DebugMsg("GameListItem", "Released GameListItem(%X) %s at ContentItem:  %X", this, Content->getTitle()m_Item->getTitle().c_str(), m_Item);
	// Delete our Texture Item Classes so they free themselves
	if(Textures.m_IconTexture != NULL)
		delete Textures.m_IconTexture;

	if(Textures.m_BoxartTexture != NULL)
		delete Textures.m_BoxartTexture;

	if(Textures.m_BannerTexture != NULL)
		delete Textures.m_BannerTexture;

	if(Textures.m_BkgTexture != NULL)
		delete Textures.m_BkgTexture;

	if(Textures.m_SS1Texture != NULL)
		delete Textures.m_SS1Texture;

	if(Textures.m_SS2Texture != NULL)
		delete Textures.m_SS2Texture;

	Textures.m_IconTexture = NULL;
	Textures.m_BoxartTexture = NULL;
	Textures.m_BkgTexture = NULL;
	Textures.m_SS1Texture = NULL;
	Textures.m_SS2Texture = NULL;
}

GameListItem::GameListItem(ContentItemNew* item)
{
	Content.m_szGenre = NULL;
	Content.m_szTitle = NULL;
	Content.m_szDeveloper = NULL;
	Content.m_szId = NULL;
	Content.m_szMid = NULL;
	Content.m_szDescription = NULL;
	Content.m_szIcon = NULL;
	Content.m_szReleaseDate = NULL;
	Content.m_szRating = NULL;
	Content.m_szRaters = NULL;
	Content.m_szLastPlayedDate = NULL;
	Content.m_szExecutable = NULL;
	Content.m_szPath = NULL;
	Content.m_szDeviceCode = NULL;
	
	ContentRef = NULL;
	ContentRef = item;

	InitializeContentText();
	if(ContentRef != NULL)
	{
		Textures.m_IconTexture = new TextureItem();
		Textures.m_BoxartTexture = new TextureItem();
		Textures.m_BannerTexture = new TextureItem();
		Textures.m_BkgTexture = new TextureItem();
		Textures.m_SS1Texture = new TextureItem();
		Textures.m_SS2Texture = new TextureItem();
		
		//RefreshContentText();
	}
	else
	{
		Textures.m_IconTexture = NULL;
		Textures.m_BoxartTexture = NULL;
		Textures.m_BannerTexture = NULL;
		Textures.m_BkgTexture = NULL;
		Textures.m_SS1Texture = NULL;
		Textures.m_SS2Texture = NULL;
	}
}

HRESULT GameListItem::RefreshContentText()
{
	if(ContentRef != NULL)
	{
		Content.m_szId = ContentRef->getId().c_str();
		Content.m_szMid = ContentRef->getMId().c_str();
		// (These need proper conversions)
		//Content.m_szExecutable = strtowstr(ContentRef->getExecutable()).c_str();
		//Content.m_szPath = strtowstr(ContentRef->getPath()).c_str());
		Content.m_szLastPlayedDate = strtowstr(ContentRef->getLastPlayedDate()).c_str();// this .c_str probably isn't safe
		Content.m_nLastPlayedOrder = ContentRef->getLastPlayedOrder();

		Content.m_szTitle = ContentRef->getTitle().c_str();
		Content.m_szDescription = ContentRef->getDescription().c_str();
		Content.m_szGenre = ContentRef->getGenre().c_str();
		Content.m_szDeveloper = ContentRef->getDeveloper().c_str();
		Content.m_szReleaseDate = ContentRef->getReleaseDate().c_str();
		
		m_szRatingPercent = ContentRef->getRating() + L" / 5.00";
		Content.m_szRating = m_szRatingPercent.c_str();
		Content.m_szRaters = ContentRef->getRaters().c_str();
	
		// Set up User Rating
		Content.m_vUserRating.clear();
		Content.m_nUserRating = ContentRef->getUserRating();//(int)floor(_wtof(ContentRef->getRating().c_str()) * 100);

		int nFullStarCount = (int)floor((double)(Content.m_nUserRating / 100));	// integer representing number of full stars
		int nPartialStarIndex = (int)floor((double)((Content.m_nUserRating % 100) / 25)); // integer representing index for partial star

		for(int nIndex = 0; nIndex < 5; nIndex++)
		{
			if(nIndex < nFullStarCount)
				Content.m_vUserRating.push_back(4);
			else if(nPartialStarIndex > 0 && nIndex == nFullStarCount)
				Content.m_vUserRating.push_back(nPartialStarIndex);
			else
				Content.m_vUserRating.push_back(0);
		}

		// Set up Device Code
	
		m_szAltText = make_uppercaseW(strtowstr(ContentRef->getRoot()) + L": " + ContentRef->getId());
		Content.m_szDeviceCode = m_szAltText.c_str();
		
		Flags.m_bFavorite = ContentRef->getIsFavorite() == TRUE ? true : false;

		return S_OK;
	}

	return S_FALSE;
}

HRESULT GameListItem::InitializeContentText()
{
	Content.m_szId = L"";
	Content.m_szLastPlayedDate = L"";
	Content.m_nLastPlayedOrder = 0;

	if(ContentRef != NULL)
		Content.m_szTitle = ContentRef->getTitle().c_str();
	else 
		Content.m_szTitle = L"Not Available";

	Content.m_szDescription = L"";
	Content.m_szGenre = L"";
	Content.m_szDeveloper = L"";
	Content.m_szReleaseDate = L"";
	
	m_szRatingPercent = L"";
	Content.m_szRating = L"";
	Content.m_szRaters = L"";

	// Set up User Rating
	Content.m_vUserRating.clear();
	Content.m_nUserRating = 0;

	for(int nIndex = 0; nIndex < 5; nIndex++)
		Content.m_vUserRating.push_back(0);

	m_szAltText = L"";
	Content.m_szDeviceCode = L"";
	
	Flags.m_bFavorite = false;

	return S_FALSE;
}