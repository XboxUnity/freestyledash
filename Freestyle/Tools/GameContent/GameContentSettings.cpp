#pragma once
#include "stdafx.h"
#include "GameContentManager.h"
#include "../Managers/Skin/SkinXMLReader/SkinXMLReader.h"
#include "../Settings/Settings.h"
#include "../SQLite/FSDSql.h"

HRESULT GameContentManager::ReadUserSettings( void )
{
	m_bDisplayBackground = SETTINGS::getInstance().getUseBackground() == 1 ? true : false;
	m_bDisplayScreenshots = SETTINGS::getInstance().getUseScreenshots() == 1 ? true : false; 
	//m_bDisplayVideos = SETTINGS::getInstance().getUsePreviews() == 1 ? true : false;

	int nSortStyle, nSortDirection, nSubTitleView;
//	SETTINGS::getInstance().getGameListSort((int)m_sTabId, &nSortStyle, &nSortDirection, &nSubTitleView);

	// get the sort settings from the database
	FSDSql::getInstance().getGameListSort((int)m_sTabId, 0, nSortStyle, nSortDirection, nSubTitleView, m_bDisplayFavorites);

	m_sListState.SortDirection = (GAMELIST_SORTDIRECTION)nSortDirection;
	m_sListState.SortStyle = (GAMELIST_SORTSTYLE)nSortStyle;
	m_sListState.SubTitleView = (GAMELIST_SUBTITLEVIEW)nSubTitleView;

	return S_OK;
}

HRESULT GameContentManager::ReadSkinSettings( void )
{
	SkinXMLReader skinXml;
	skinXml.LoadSettings("GamesList", "GameList");
	//skinXml.LoadSettings("GameContentManager", "GameContentManager");

	// Get Path for Missing Images from active skin.xml
	m_sListSettings.MissingIconPath = skinXml.GetSetting("MISSINGICONPATH", "");
	m_sListSettings.MissingBoxartPath = skinXml.GetSetting("MISSINGBOXARTPATH", "");
	m_sListSettings.MissingBannerPath = skinXml.GetSetting("MISSINGBANNERPATH", "");
	m_sListSettings.MissingScreenshotPath = skinXml.GetSetting("MISSINGSCREENSHOTPATH", "");

	// Get Path for Loading Images from active skin.xml
	m_sListSettings.LoadingIconPath = skinXml.GetSetting("LOADINGICONPATH", "");
	m_sListSettings.LoadingBoxartPath = skinXml.GetSetting("LOADINGBOXARTPATH", "");
	m_sListSettings.LoadingBannerPath = skinXml.GetSetting("LOADINGBANNERPATH", "");
	m_sListSettings.LoadingScreenshotPath = skinXml.GetSetting("LOADINGSCREENSHOTPATH", "");

	// Get Path for the Favorites Star from active skin.xml
	m_sListSettings.StarFavoritePath = skinXml.GetSetting("FAVORITESTARPATH", "");

	// Get Path for the Rating Stars from active skin.xml
	m_sListSettings.StarRatingPath[0] = skinXml.GetSetting("RATINGSTAR000PATH", "");
	m_sListSettings.StarRatingPath[1] = skinXml.GetSetting("RATINGSTAR025PATH", "");
	m_sListSettings.StarRatingPath[2] = skinXml.GetSetting("RATINGSTAR050PATH", "");
	m_sListSettings.StarRatingPath[3] = skinXml.GetSetting("RATINGSTAR075PATH", "");
	m_sListSettings.StarRatingPath[4] = skinXml.GetSetting("RATINGSTAR100PATH", "");

	return S_OK;
}