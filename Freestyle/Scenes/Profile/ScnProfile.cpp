#include "stdafx.h"

#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Monitors/SignIn/SignInMonitor.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Application/FreestyleApp.h"
#include "../../Tools/SQLite/FSDSql.h"
#include "../../Tools/ContentList/ContentManager.h"
#include "../../Tools/Texture/TextureCache.h"

#include "ScnProfile.h"

using namespace std;

CScnProfile::CScnProfile()
{
	// Initialize Private State Variables
	ProfileSceneSettings.m_bProfileLoggedIn = false;
	ProfileSceneSettings.szGamerScore = L"0";
	ProfileSceneSettings.szGamerTag = L"";

	// Initalize the Titles Played Vector
	vTitlesPlayed.clear();

	// Initialize Skin.Xml Settings
	ProfileSceneSettings.m_nPlayerIndex = 0;
	ProfileSceneSettings.m_bRenderGamerPic = false;
	ProfileSceneSettings.m_bRenderRecent = false;
	
	// Initialize the presenter references
	ProfileSceneSettings.m_bGamerPicError = false;
	for(int nCount = 0; nCount < MAX_RECENTGAME; nCount++)
		ProfileSceneSettings.m_bRecentGameError[nCount] = false;
}

CScnProfile::~CScnProfile()
{
	// Unregister Scene from the SignInMonitor
	DebugMsg("ScnProfile", "Destroyed");
	SignInMonitor::getInstance().RemoveObserver(*this);
}

HRESULT CScnProfile::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	// Read Settings from Skin.xml
	ReadSettings();

	// Initialize Scene XUI Controls
	InitializeChildren();

	// Create Textures for GamerPic
	if(CreateGamerPicTexture() == S_FALSE)
		ProfileSceneSettings.m_bGamerPicError = true;

	#ifndef USE_RECENT_FROM_DB
		// Create Textures for Recent Games
		for(int nCount = 0; nCount < MAX_RECENTGAME; nCount++) {
			if(CreateRecentGameTextures(nCount) == S_FALSE)
				ProfileSceneSettings.m_bRecentGameError[nCount] = true;
		}
	#endif

	// Hide Scene if HideMode is turned on
	if(m_HideMode != 0)
		Hide();

	// Hide Kai Online Status untill SignedIn
	if(SceneXuiControlState.hasKaiSignedInLabel)
	{
		SceneXuiControls.m_SignedIntoKaiLabel.SetShow(false);
	}

	if(SceneXuiControlState.hasKaiSignedIn)
	{
		SceneXuiControls.m_SignedIntoKai.SetShow(false);
	}

	// Update all class information and prepare for display
	Update();

	// Register with the SignInMonitor
	SignInMonitor::getInstance().AddObserver(*this);
    return S_OK;
}

HRESULT CScnProfile::ReadSettings( void )
{
	string strSetting = "";

	// Load settings from skin.xml
	LoadSettings("ScnProfile", *this);

	// Read XML to obtain which Player to display
	strSetting = GetSetting("PLAYERID", "1");
	ProfileSceneSettings.m_nPlayerIndex = atoi(strSetting.c_str()) - 1;

	strSetting = GetSetting("RENDERRECENT", "TRUE");
	ProfileSceneSettings.m_bRenderRecent = strcmp(strSetting.c_str(), "FALSE") == 0 ? false : true;

	strSetting = GetSetting("RENDERGAMERPIC", "TRUE");
	ProfileSceneSettings.m_bRenderGamerPic = strcmp(strSetting.c_str(), "FALSE") == 0 ? false : true;

	return S_OK;
}

HRESULT CScnProfile::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,  BOOL& bHandled)
{
	// If there are no profiles currently logged in, return
	if(ProfileSceneSettings.m_bProfileLoggedIn == false){ 
		bHandled = true;
		return S_OK;
	}

	if(PROFILEIMG_GAMERPIC == pGetSourceImageData->iData && !pGetSourceImageData->bItemData) {
		if(ProfileSceneSettings.m_bRenderGamerPic && Textures.m_GamerPictureTexture.GetCurrentBrushState() == FSDTEX_BRUSHLOADED)
			pGetSourceImageData->hBrush = Textures.m_GamerPictureTexture.GetTextureBrush();
		else
			pGetSourceImageData->hBrush = NULL;

		bHandled = true;
	}

	for(int i = 0; i < MAX_RECENTGAME; i++) {
		if(PROFILEIMG_RECENTGAMEBASE + i == pGetSourceImageData->iData && !pGetSourceImageData->bItemData) {
			if(ProfileSceneSettings.m_bRenderRecent && Textures.m_RecentGameTexture[i].GetCurrentBrushState() == FSDTEX_BRUSHLOADED)
				pGetSourceImageData->hBrush = Textures.m_RecentGameTexture[i].GetTextureBrush();
			else
				pGetSourceImageData->hBrush = NULL;

			bHandled = true;
		}
	}
	return S_OK;
}

void CScnProfile::Update()
{
	DebugMsg("ScnProfile", "Notified of profile change... THIS:  %X", this);

	if(!IsUserSignedIn(ProfileSceneSettings.m_nPlayerIndex))
	{
		if(m_HideMode != HIDEMODE_TRANSITION)
		{
			// If HideMode is not set to Transition, hide Xui Elements.
			DebugMsg("ScnProfile", "Hiding Profile Scene.  HIDEMODE: %d", m_HideMode);

			if(SceneXuiControlState.hasGamerScore)
				SceneXuiControls.m_GamerScore.SetText(L"");
			
			if(SceneXuiControlState.hasGamerTag)
				SceneXuiControls.m_GamerTag.SetText(L"");

			#ifdef USE_RECENT_FROM_DB
				// Before we begin, we need to release any and all textures that are existing
				for(DWORD nIndex = 0; nIndex < MAX_RECENTGAME; nIndex++)
				{
					Textures.m_RecentGameTexture[nIndex].Release();
					Textures.m_RecentGameTexture[nIndex].SetRetainSource(TRUE);
					Textures.m_RecentGameTexture[nIndex].ReleaseAll();
				}

				// Erase existing ContentItems
				for(DWORD nIndex = 0; nIndex < vRecentContentItems.size(); nIndex++)
				{
					ContentItemNew * pRecentContent = vRecentContentItems.at(nIndex);
					if(pRecentContent != NULL)
						delete pRecentContent;
				}

				vRecentContentItems.clear();
			#endif

			Hide();
		}

		ProfileSceneSettings.m_bProfileLoggedIn = false;
	}
	else
	{
		DebugMsg("ScnProfile", "Showing Profile Scene.");
		Show();

		if(SceneXuiControlState.hasGamerScore)
		{
			// If the GamerScore XUI Element Exists, fill out the data			
			long nGamerPoints = GetGamerPoints(ProfileSceneSettings.m_nPlayerIndex);
			ProfileSceneSettings.szGamerScore = sprintfaW(L"%d", nGamerPoints);
			SceneXuiControls.m_GamerScore.SetText(ProfileSceneSettings.szGamerScore.c_str());
		}
		
		if(SceneXuiControlState.hasGamerTag)
		{
			char szTempName[MAX_GAMERTAG_LENGTH];
			XUserGetName(ProfileSceneSettings.m_nPlayerIndex, szTempName, MAX_GAMERTAG_LENGTH);
			ProfileSceneSettings.szGamerTag = strtowstr(szTempName);
			SceneXuiControls.m_GamerTag.SetText(ProfileSceneSettings.szGamerTag.c_str());	
		}
		
		if(SceneXuiControlState.hasGamerPic)
			ReadGamerPicTexture();

		if(ProfileSceneSettings.m_bRenderRecent) {
			#ifndef USE_RECENT_FROM_DB
				ReadRecentGameTexture();
			#else if
				ReadRecentGameTextureFromDB();
			#endif
		}

		ProfileSceneSettings.m_bProfileLoggedIn = true;
	}
	DebugMsg("ScnProfile", "Notified of profile change... completed");
}

HRESULT CScnProfile::ReadGamerPicTexture( void )
{
	// If there was an error duing the texture creation process- bail
	if(ProfileSceneSettings.m_bGamerPicError == true) {
		DebugMsg("ScnProfile", "Error:  GamerPic Read skipped due to invalid texture / brush");
		return S_FALSE;
	}
	
	// If the scene doesnt contain a gamer pic image presenter
	// or the settings are turned off- return
	if(!SceneXuiControlState.hasGamerPic || !ProfileSceneSettings.m_bRenderGamerPic ) {
		DebugMsg("ScnProfile", "Setting:  Gamer Pic not needing to be rendered.");
		return S_FALSE;
	}

	// If the texture brush was not created properly, leave
	if((Textures.m_GamerPictureTexture.GetCurrentBrushState() != FSDTEX_BRUSHREADY) &&
		(Textures.m_GamerPictureTexture.GetCurrentBrushState() != FSDTEX_BRUSHLOADED))
	{
		DebugMsg("ScnProfile", "Error:  Texture Brush was not properly prepared.");
		return S_FALSE;
	}

	DWORD retVal = NULL;
	D3DLOCKED_RECT rect;
	D3DSURFACE_DESC desc;

	retVal = Textures.m_GamerPictureTexture.GetD3DTexture()->GetLevelDesc( 0, &desc );
	if( FAILED( retVal )){
		ProfileSceneSettings.m_bGamerPicError = true;
		DebugMsg("ScnProfile", "Unable to get Texture level description.  HRESULT: 0x%08X", retVal);
	}

	retVal = Textures.m_GamerPictureTexture.GetD3DTexture()->LockRect(0, &rect, NULL, 0);
	if( FAILED( retVal )){
		ProfileSceneSettings.m_bGamerPicError = true;
		DebugMsg("ScnProfile", "Unable to lock Rect.  HRESULT:  0x%08X", retVal);
	}

	retVal = GetGamerPicture(ProfileSceneSettings.m_nPlayerIndex, (BYTE*)rect.pBits, false, rect.Pitch, desc.Height);
	if(FAILED( retVal )){
		ProfileSceneSettings.m_bGamerPicError = true;
		DebugMsg("ScnProfile", "Unable to retrieve GamerPic.  DWORD: 0x%08X", retVal);
	}
	else
	{
		Textures.m_GamerPictureTexture.SetCurrentBrushState(FSDTEX_BRUSHLOADED);
	}

	Textures.m_GamerPictureTexture.GetD3DTexture()->UnlockRect( 0 );

	return ProfileSceneSettings.m_bGamerPicError == true ? S_OK : S_FALSE;
}

HRESULT CScnProfile::ReadRecentGameTextureFromDB( void )
{
	// Before we begin, we need to release any and all textures that are existing
	for(DWORD nIndex = 0; nIndex < MAX_RECENTGAME; nIndex++)
	{
		Textures.m_RecentGameTexture[nIndex].Release();
		Textures.m_RecentGameTexture[nIndex].SetRetainSource(TRUE);
		Textures.m_RecentGameTexture[nIndex].ReleaseAll();
	}

	//DebugMsg("ScnProfileDEBUG", "Cleared Existing Textures");

	// Erase existing ContentItems
	for(DWORD nIndex = 0; nIndex < vRecentContentItems.size(); nIndex++)
	{
		ContentItemNew * pRecentContent = vRecentContentItems.at(nIndex);
		if(pRecentContent != NULL)
			delete pRecentContent;
	}

	//DebugMsg("ScnProfileDEBUG", "Cleared Existing Content Items");

	vRecentContentItems.clear();

	vTitlesPlayed.clear();
	vTitlesPlayed = FSDSql::getInstance().getMostRecentlyPlayedIds( ProfileSceneSettings.m_nPlayerIndex, MAX_RECENTGAME );

	//DebugMsg("ScnProfileDEBUG", "Obtained TitlesPlayed List from DB:  %d Titles Found", vTitlesPlayed.size());
	// First let's determine how many titles we found in the database
	DWORD dwTitleCount = vTitlesPlayed.size();

	// Next check to make sure our dwTitleCount isn't larger than our MAX_RECENTGAME
	if(dwTitleCount > MAX_RECENTGAME) dwTitleCount = MAX_RECENTGAME;

	//DebugMsg("ScnProfileDEBUG", "Entering TExture Loading Loop....");
	for(DWORD nIndex = 0; nIndex < dwTitleCount; nIndex++) 
	{
		// First let's check and see if we have the image presenter available for display
		if(SceneXuiControlState.hasRecentGame[nIndex] == false) {
			//DebugMsg("ScnProfileDEBUG", "Skin is missing TexturePresenter #:  %d", nIndex);
			continue;
		}
		
		DWORD dwContentId = (DWORD)vTitlesPlayed.at(nIndex).contentId;
		//DebugMsg("ScnProfileDEBUG", "Currently Processing Content Item[%d] with Content Id:  %X", nIndex, dwContentId);

		CONTENT_ITEM_LOAD_INFO ItemData;
		FSDSql::getInstance().LoadContentItemFromContentId(dwContentId, ItemData);
		if(ItemData.ContentId == 0) {
			//DebugMsg("ScnProfileDEBUG", "Content Id Returned as 0, skipping item-  nIndex = %d", nIndex);
			continue;
		}

		ContentItemNew * pContentItem = new ContentItemNew(ItemData);
		//DebugMsg("ScnProfileDEBUG", "Content Item Retrived.... Checking Title for validity... [%s]", wstrtostr(pContentItem->getTitle()).c_str());
		vRecentContentItems.push_back(pContentItem);

		BYTE * iconData = NULL; DWORD iconSize = 0;
		iconSize = pContentItem->getIconData(&iconData);

		//DebugMsg("ScnProfielDEBUG", "nIndex = %d, IconSize = %d", nIndex, iconSize );

		// Set up the Texture Item File Data
		Textures.m_RecentGameTexture[nIndex].SetTextureInfoMemory(D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, TRUE);
		Textures.m_RecentGameTexture[nIndex].SetFilePointer(iconData, iconSize);
		Textures.m_RecentGameTexture[nIndex].AddRef();

		//DebugMsg("ScnProfileDEBUG",  "Setting texture data for index %d", nIndex);

		// Now Let's Create and Store our Texture
		IDirect3DTexture9 * pTexture = Textures.m_RecentGameTexture[nIndex].GetD3DTexture();
		HRESULT hr = TextureCache::LoadD3DTextureFromFileInMemory( iconData, iconSize, D3DX_DEFAULT_NONPOW2,
			D3DX_DEFAULT_NONPOW2, &pTexture, true);

		if( hr == S_OK )
		{
			//DebugMsg("ScnProfileDEBUG",  "Texture Loaded properly for index %d", nIndex);
			Textures.m_RecentGameTexture[nIndex].SetD3DTexture(pTexture);

			// Now Let's Create and Store our Texture Brush
			if(pTexture != NULL) {
				HXUIBRUSH hBrush = NULL;
				XuiAttachTextureBrush(pTexture, &hBrush);
				Textures.m_RecentGameTexture[nIndex].SetTextureBrush(hBrush);
				//DebugMsg("ScnProfileDEBUG",  "Brush Loaded properly for index %d", nIndex);
			}
		}
		else
		{
			DebugMsg("ScnProfileDEBUG",  "Texture failed to load properly for index %d", nIndex);
		}
	}

	return S_OK;
}

HRESULT CScnProfile::ReadRecentGameTexture( void )
{
	// If the scene does not require any rendering
	if(!ProfileSceneSettings.m_bRenderRecent ) {
		DebugMsg("ScnProfile", "Setting:  Recent Games not needing to be rendered.");
		for(int nCount = 0; nCount < MAX_RECENTGAME; nCount)
			ProfileSceneSettings.m_bRecentGameError[nCount] = true;

		return S_FALSE;
	}
	
	DWORD dwBufferSize = 0;
	HANDLE fHandle = NULL;

	DWORD retVal = XamUserCreateTitlesPlayedEnumerator(0, ProfileSceneSettings.m_nPlayerIndex, INVALID_XUID, 0, 0x100, &dwBufferSize, &fHandle);
	if(retVal != ERROR_SUCCESS) {
		DebugMsg("ScnProfile", "Error:  CreateTitlesPlayedEnumerator Failed.  HRESULT:  0x%08X", retVal);
		for(int nCount = 0; nCount < MAX_RECENTGAME; nCount++)
			ProfileSceneSettings.m_bRecentGameError[nCount] = true;

		return S_FALSE;
	}

	DWORD dwItemCount = 0;
	PXUSER_TITLE_PLAYED pTitles = (PXUSER_TITLE_PLAYED)malloc(dwBufferSize);
	retVal = XEnumerate(fHandle, pTitles, dwBufferSize, &dwItemCount, NULL);
	if(retVal != ERROR_SUCCESS) {
		DebugMsg("ScnProfile", "Error:  XEnumerate Failed.  HRESULT:  0x%08X", retVal);
		for(int nCount = 0; nCount < MAX_RECENTGAME; nCount++)
			ProfileSceneSettings.m_bRecentGameError[nCount] = true;

		return S_FALSE;
	}

	// Enumerated Successfully - proceed to obtaining correct textures
	vector<PlayedTitle> vTitlesPlayed;

	// Fill a vector containing all of the titles
	for(int nTitleCount = 0; nTitleCount < (int)dwItemCount; nTitleCount++) {
		PlayedTitle plTitle;
		plTitle.dwTitleId = pTitles[nTitleCount].dwTitleId;
		plTitle.ftLastLoaded = pTitles[nTitleCount].ftLastLoaded;
		DebugMsg("DEBUG", "High:  %X,  Low:  %X", plTitle.ftLastLoaded.dwHighDateTime, plTitle.ftLastLoaded.dwLowDateTime);
		vTitlesPlayed.push_back(plTitle);
	}

	// Sort the list to put the most recently played games into the beginning
	sort(vTitlesPlayed.begin(), vTitlesPlayed.end(), sortPlayeTitleLastLoaded);

	// Proceed to loading texture data, but first make sure we set our max textures
	int nMaxTitles = vTitlesPlayed.size() < MAX_RECENTGAME+1 ? vTitlesPlayed.size() : MAX_RECENTGAME;

	for(int nCount = 0; nCount < nMaxTitles; nCount++) {
		if(SceneXuiControlState.hasRecentGame[nCount] && !ProfileSceneSettings.m_bRecentGameError[nCount]) {
			DWORD dwTitleId = vTitlesPlayed.at(nCount).dwTitleId;
			DebugMsg("Test" , "Title:  %X,  High:  %X,  Low:  %X", dwTitleId, vTitlesPlayed.at(nCount).ftLastLoaded.dwHighDateTime, vTitlesPlayed.at(nCount).ftLastLoaded.dwLowDateTime);
		
			DWORD retVal = NULL;
			D3DLOCKED_RECT rect;
			D3DSURFACE_DESC desc;

			retVal = Textures.m_RecentGameTexture[nCount].GetD3DTexture()->GetLevelDesc( 0, &desc );
			if( FAILED( retVal )){
				ProfileSceneSettings.m_bRecentGameError[nCount] = true;
				DebugMsg("ScnProfile", "Unable to get Texture level description.  HRESULT: 0x%08X", retVal);
			}

			retVal = Textures.m_RecentGameTexture[nCount].GetD3DTexture()->LockRect(0, &rect, NULL, 0);
			if( FAILED( retVal )){
				ProfileSceneSettings.m_bRecentGameError[nCount] = true;
				DebugMsg("ScnProfile", "Unable to lock Rect.  HRESULT:  0x%08X", retVal);
			}

			retVal = XamReadTileToTexture(0, dwTitleId, TILEID, ProfileSceneSettings.m_nPlayerIndex, (BYTE*)rect.pBits, rect.Pitch, desc.Height, NULL);
			if(FAILED( retVal )){
				ProfileSceneSettings.m_bRecentGameError[nCount] = true;
				DebugMsg("ScnProfile", "Unable to retrieve Game Icon.  DWORD: 0x%08X", retVal);
			}
			else
			{
				Textures.m_RecentGameTexture[nCount].SetCurrentBrushState(FSDTEX_BRUSHLOADED);
			}

			Textures.m_RecentGameTexture[nCount].GetD3DTexture()->UnlockRect( 0 );

		}
		// Texture Error or Missing Image Presenter- Skip
	}

	return S_OK;
}

HRESULT CScnProfile::CreateRecentGameTextures(int nGameIndex)
{
	// Obtain Direct 3D Device for Texture Creation
	IDirect3DDevice9 * pDevice = CFreestyleApp::getInstance().m_pd3dDevice;
	
	// Don't create texture if GamerPic rendering is turned off
	if(ProfileSceneSettings.m_bRenderRecent == false) {
		DebugMsg("ScnProfile", "Setting:  Creating Texture[%d] Skipped- RecentGame Rendering Off", nGameIndex);
		return S_FALSE;
	}

	// If the scene does not contain a Gamerpic Image Presenter - then don't create texture
	if(SceneXuiControlState.hasRecentGame[nGameIndex] != true) return S_FALSE;

	// All conditions are requiring us to create a texture
	IDirect3DTexture9 * pTexRecentGame = Textures.m_RecentGameTexture[nGameIndex].GetD3DTexture();
	Textures.m_RecentGameTexture[nGameIndex].SetCurrentTextureState(FSDTEX_TEXINIT);

	// 
	
	// Create the empty texture
	HRESULT retVal = pDevice->CreateTexture(TILEWIDTH, TILEHEIGHT, 1, 0, 
		D3DFMT_LIN_A8R8G8B8, 0, &pTexRecentGame, NULL);

	// Texture Failed To Create
	if(retVal != ERROR_SUCCESS) {
		DebugMsg("ScnProfile", "Error:  RecentGame[%d] Texture Creation Failed.  HRESULT:  %08X", nGameIndex, retVal);
		Textures.m_RecentGameTexture[nGameIndex].SetCurrentTextureState(FSDTEX_TEXERROR);
		return S_FALSE;
	}
	
	// Texture Created Successfully
	Textures.m_RecentGameTexture[nGameIndex].AddRef();
	Textures.m_RecentGameTexture[nGameIndex].SetD3DTexture(pTexRecentGame);
	Textures.m_RecentGameTexture[nGameIndex].SetCurrentTextureState(FSDTEX_TEXREADY);

	// Create the XuiTextureBrush
	HXUIBRUSH hBrush = NULL;
	retVal = XuiAttachTextureBrush(pTexRecentGame, &hBrush);
	if(retVal != S_OK){
		DebugMsg("ScnProfile", "Error:  RecentGame[%d] Brush Creation Failed.  HRESULT:  %08X", nGameIndex, retVal);	
		// Safely release our texture if the Texture Brush failed to create
		if(pTexRecentGame != NULL) 
			pTexRecentGame->Release();
		pTexRecentGame = NULL;

		Textures.m_RecentGameTexture[nGameIndex].SetCurrentBrushState(FSDTEX_BRUSHERROR);
		Textures.m_RecentGameTexture[nGameIndex].SetCurrentTextureState(FSDTEX_TEXERROR);
		Textures.m_RecentGameTexture[nGameIndex].Release();

		return S_FALSE;
	}

	// Texture Brush created successfully
	Textures.m_RecentGameTexture[nGameIndex].SetTextureBrush(hBrush);
	Textures.m_RecentGameTexture[nGameIndex].SetCurrentBrushState(FSDTEX_BRUSHREADY);

	return S_OK;
}

HRESULT CScnProfile::CreateGamerPicTexture()
{	
	// Obtain Direct 3D Device for Texture Creation
	IDirect3DDevice9 * pDevice = CFreestyleApp::getInstance().m_pd3dDevice;
	
	// Don't create texture if GamerPic rendering is turned off
	if(ProfileSceneSettings.m_bRenderGamerPic == false) {
		DebugMsg("ScnProfile", "Setting:  Creating Texture Skipped- GamerPic Rendering Off");
		return S_FALSE;
	}

	// If the scene does not contain a Gamerpic Image Presenter - then don't create texture
	if(SceneXuiControlState.hasGamerPic != true) return S_FALSE;

	// All conditions are requiring us to create a texture
	IDirect3DTexture9 * pTexGamerPic = Textures.m_GamerPictureTexture.GetD3DTexture();
	Textures.m_GamerPictureTexture.SetCurrentTextureState(FSDTEX_TEXINIT);

	// Create the empty texture
	HRESULT retVal = pDevice->CreateTexture(TILEWIDTH, TILEHEIGHT, 1, 0, 
		D3DFMT_LIN_A8R8G8B8, 0, &pTexGamerPic, NULL);

	// Texture Failed To Create
	if(retVal != ERROR_SUCCESS) {
		DebugMsg("ScnProfile", "Error:  Gamer Pic Texture Creation Failed.  HRESULT:  %08X", retVal);
		Textures.m_GamerPictureTexture.SetCurrentTextureState(FSDTEX_TEXERROR);
		return S_FALSE;
	}
	
	// Texture Created Successfully
	Textures.m_GamerPictureTexture.AddRef();
	Textures.m_GamerPictureTexture.SetD3DTexture(pTexGamerPic);
	Textures.m_GamerPictureTexture.SetCurrentTextureState(FSDTEX_TEXREADY);

	// Create the XuiTextureBrush
	HXUIBRUSH hBrush = NULL;
	retVal = XuiAttachTextureBrush(pTexGamerPic, &hBrush);
	if(retVal != S_OK){
		DebugMsg("ScnProfile", "Error:  Gamer Pic Brush Creation Failed.  HRESULT:  %08X", retVal);	
		// Safely release our texture if the Texture Brush failed to create
		if(pTexGamerPic != NULL) 
			pTexGamerPic->Release();
		pTexGamerPic = NULL;

		Textures.m_GamerPictureTexture.SetCurrentBrushState(FSDTEX_BRUSHERROR);
		Textures.m_GamerPictureTexture.SetCurrentTextureState(FSDTEX_TEXERROR);
		Textures.m_GamerPictureTexture.Release();

		return S_FALSE;
	}

	// Texture Brush created successfully
	Textures.m_GamerPictureTexture.SetTextureBrush(hBrush);
	Textures.m_GamerPictureTexture.SetCurrentBrushState(FSDTEX_BRUSHREADY);

	return S_OK;
}


HRESULT CScnProfile::InitializeChildren( void )
{
	for(int nCount = 0; nCount < MAX_RECENTGAME; nCount++){
		wstring szControlId = sprintfaW(L"RecentGame%d", nCount + 1);
		XuiInitControl(szControlId.c_str(), SceneXuiControls.m_RecentGame[nCount], &SceneXuiControlState.hasRecentGame[nCount]);
	}

	XuiInitControl(L"GamerPic", SceneXuiControls.m_GamerPic, &SceneXuiControlState.hasGamerPic);
	XuiInitControl(L"GamerTag", SceneXuiControls.m_GamerTag, &SceneXuiControlState.hasGamerTag);
	XuiInitControl(L"GamerScore", SceneXuiControls.m_GamerScore, &SceneXuiControlState.hasGamerScore);
	XuiInitControl(L"GamerScoreIcon", SceneXuiControls.m_GamerScoreIcon, &SceneXuiControlState.hasGamerScoreIcon);
	XuiInitControl(L"KaiSignedIn", SceneXuiControls.m_SignedIntoKai, &SceneXuiControlState.hasKaiSignedIn);
	XuiInitControl(L"KaiSignedInLabel", SceneXuiControls.m_SignedIntoKaiLabel, &SceneXuiControlState.hasKaiSignedInLabel);

	return S_OK;
}

HRESULT CScnProfile::OnNotifyAccountLoggedIn( KAIMessageOnAccountLoggedIn * pOnAccountLoggedIn, BOOL& bHandled )
{
	if(SceneXuiControlState.hasKaiSignedInLabel)
	{
		SceneXuiControls.m_SignedIntoKaiLabel.SetShow(true);
	}

	if(SceneXuiControlState.hasKaiSignedIn)
	{
		SceneXuiControls.m_SignedIntoKai.SetShow(true);
	}
	return S_OK;
}

HRESULT CScnProfile::OnNotifyDeinitialise( KAIMessageOnDeinitialise * pOnDeinitialise, BOOL& bHandled )
{
	if(SceneXuiControlState.hasKaiSignedInLabel)
	{
		SceneXuiControls.m_SignedIntoKaiLabel.SetShow(false);
	}

	if(SceneXuiControlState.hasKaiSignedIn)
	{
		SceneXuiControls.m_SignedIntoKai.SetShow(false);
	}

	return S_OK;
}

bool sortPlayeTitleLastLoaded(PlayedTitle left, PlayedTitle right)
{
	return CompareFileTime(&(right.ftLastLoaded), &(left.ftLastLoaded)) < 0 ? true : false;
}