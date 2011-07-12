#pragma once

#include "../Abstracts/ConfigurableScene/ConfigurableScene.h"
#include "../Abstracts/HideableScene/HideableScene.h"
#include "../../Tools/HTTP/HttpItem.h"
#include "../../Tools/Generic/tools.h"
#include "../../Tools/Generic/xboxtools.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Settings/Settings.h"
#include "../../Tools/HTTP/HTTPDownloader.h"
#include "../../Tools/XML/XMLReader.h"
#include "../../Tools/AutoUpdater/AutoUpdater.h"


class CScnUpdater : public HideableScene, public iUpdateDisplayer
{
private :
	HRESULT InitializeChildren( );
	HRESULT CheckForUpdate();
	HRESULT ProcessCurrentVersionXML(HttpItem *itm);

	void FillList();
	bool CompareVersions(VersionDescriptor versionA, VersionDescriptor versionB, bool bIncludeBeta);
	void DownloadSelectedUpdates();
	void CreateDownloadQueue( ListGroup * dashQueue, ListGroup * skinQueue, ListGroup * pluginQueue);

	VersionDescriptor dashVersion;
	VersionDescriptor skinVersion;
	VersionDescriptor curDashVersion;
	VersionDescriptor curSkinVersion;

	Header xmlHeader;
	vector<VersionDescriptor> m_DashVersions;
	vector<VersionDescriptor> m_SkinVersions;
	vector<VersionDescriptor> m_PluginVersions;
	vector<VersionDescriptor> m_DownloadList;

	VersionDescriptor * tempVersion;

	bool bUpdatedXexNotice;
	bool bUpdatedSkinNotice;
	bool bUpdatedBetaNotice;

protected :

	CXuiList m_DashUpdates;
	CXuiList m_SkinUpdates;
	CXuiList m_PluginUpdates;
	CXuiControl m_PleaseWait;
	CXuiTextElement m_UpdateStatus;
	CXuiControl m_ApplyUpdates;
	CXuiControl m_ShowChangelog;
	CXuiControl m_SelectUpdates;
	CXuiTextElement m_DashStatus;
	CXuiTextElement m_SkinStatus;
	CXuiTextElement m_PluginStatus;
	CXuiEdit m_ChangeLogEdit;

	bool hasDashUpdates, hasSkinUpdates, hasPluginUpdates;
	bool hasDashStatus, hasSkinStatus, hasPluginStatus;
	bool hasChangeLogEdit;
	bool hasUpdateStatus;
	bool hasApplyUpdates;
	bool hasPleaseWait;
	bool hasShowChangelog;

	bool bChangelogMode;

public :
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_KILL_FOCUS( OnNotifyKillFocus )
		XUI_ON_XM_SET_FOCUS( OnSetFocus )
	XUI_END_MSG_MAP()

	CScnUpdater();
	~CScnUpdater();
	XUI_IMPLEMENT_CLASS(CScnUpdater, L"ScnUpdater", XUI_CLASS_TABSCENE);
	
	HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnSetFocus(HXUIOBJ hObjLosingFocus, BOOL& bHandled);
	HRESULT OnNotifyKillFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled );

	void handleStatusUpdate( string szUpdate );
	void handleDownloadFinished( int nItem );
	void handleLoadingComplete( void );
	void ShowButtons();
	void HideButtons();

};