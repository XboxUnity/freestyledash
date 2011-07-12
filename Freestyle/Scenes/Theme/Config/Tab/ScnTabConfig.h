#pragma once
#include "../../../../Tools/Generic/tools.h"
#include "../../../../Tools/Managers/Theme/TabManager/TabManager.h"
#include "../../../../Tools/Threads/ThreadLock.h"

#include "TabConfigHub.h"

class CScnTabConfig : public CXuiTabSceneImpl, public iTabConfigObserver
{
protected:
	//Background Selection Controls
	CXuiList m_BkgList;
	CXuiImageElement m_BkgPreview;
	CXuiTextElement m_BkgName;
	CXuiTextElement m_BkgListCounter;
	CXuiTextElement m_BkgListCounterTotal;
	CXuiCheckbox m_ShowGradient;
	CXuiCheckbox m_ShowAnimation;
	bool hasBkgList, hasBkgPreview, hasBkgName, hasShowGradient, hasShowAnimation, hasBkgListCounter, hasBkgListCounterTotal;

	//Icon Selection Controls
	CXuiList m_IconList;
	CXuiImageElement m_IconPreview;
	CXuiTextElement m_IconName;
	CXuiTextElement m_IconListCounter;
	CXuiTextElement m_IconListCounterTotal;
	bool hasIconList, hasIconPreview, hasIconName, hasIconListCounter, hasIconListCounterTotal;

	//Reflection Selection Controls
	CXuiList m_ReflectionList;
	CXuiImageElement m_ReflectionPreview;
	CXuiTextElement m_ReflectionName;
	CXuiTextElement m_ReflectionListCounter;
	CXuiTextElement m_ReflectionListCounterTotal;
	bool hasReflectionList, hasReflectionPreview, hasReflectionName, hasReflectionListCounter, hasReflectionListCounterTotal;

	//TextColor Selection Controls
	CXuiTextElement m_MainRedValue, m_MainGreenValue, m_MainBlueValue, m_MainAlphaValue;
	CXuiTextElement m_ReflectRedValue, m_ReflectGreenValue, m_ReflectBlueValue, m_ReflectAlphaValue;
	CXuiSlider m_MainRed, m_MainGreen, m_MainBlue, m_MainAlpha;
	CXuiSlider m_ReflectRed, m_ReflectGreen, m_ReflectBlue, m_ReflectAlpha;
	bool hasMainRedValue, hasMainGreenValue, hasMainBlueValue, hasMainAlphaValue;
	bool hasMainRed, hasMainGreen, hasMainBlue, hasMainAlpha;
	bool hasReflectRedValue, hasReflectGreenValue, hasReflectBlueValue, hasReflectAlphaValue;
	bool hasReflectRed, hasReflectGreen, hasReflectBlue, hasReflectAlpha;

	//Global Buttons and Labels
	CXuiControl m_ApplyTabChange;
	CXuiControl m_SelectTab;
	CXuiControl m_SaveChanges;
	CXuiControl m_ApplyToMenu, m_ApplyToDash;
	CXuiControl m_Randomize;

	CXuiTextElement m_RandomStatus;
	bool hasRandomStatus;

	CXuiTextElement m_ParentIndex, m_ChildIndex;
	bool hasApplyTabChange, hasSelectTab, hasSaveChanges;
	bool hasApplyToMenu, hasApplyToDash, hasRandomize;
	bool hasParentIndex, hasChildIndex;

	//Preview Controls
	CXuiImageElement m_Background, m_Reflection;
	CXuiTextElement m_MainText, m_ReflectText;
	CXuiControl m_Animation, m_Gradient, m_Icon;
	bool hasBackground, hasReflection, hasAnimation, hasGradient, hasIcon;
	bool hasMainText, hasReflectText;
	
	//Selection Display Controls
	CXuiTextElement m_BkgVisualStat, m_IconVisualStat, m_ReflectVisualStat;
	CXuiTextElement m_AnimStateStat, m_GradientStateStat, m_IconStateStat;
	CXuiTextElement m_MainTextColorStat, m_ReflectTextColorStat;

	bool hasBkgVisualStat, hasIconVisualStat, hasReflectVisualStat;
	bool hasAnimStateStat, hasGradientStateStat, hasIconStateStat;
	bool hasMainTextColorStat, hasReflectTextColorStat;

	CXuiTextElement m_VisualName, m_VisualDescription;
	bool hasVisualName, hasVisualDescription;

	//Thread Control
	ThreadLock m_Lock;

private:
	void InitializeChildren( void );
	void FillBackgroundList();
	void FillIconList();
	void FillReflectionList();
	void CreateInitialPreview();
	void SetCurrentItems( void );
	void ChangeMainColor(BYTE value, int nFlag);
	void ChangeReflectColor(BYTE value, int nFlag);
	void UpdateTabPreview(TabDataPackage tabData);
	void UpdateSelectedStats(TabDataPackage tabData);

	int focusParent, focusChild;

public:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )
		XUI_ON_XM_NOTIFY_SELCHANGED ( OnNotifySelchanged )
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
	XUI_END_MSG_MAP()

	XUI_IMPLEMENT_CLASS(CScnTabConfig,L"ScnTabConfig",XUI_CLASS_TABSCENE);

	HRESULT OnInit( XUIMessageInit *pInitData, BOOL &bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled ) ;
	HRESULT OnNotifySelchanged( HXUIOBJ hObjSource, XUINotifySelChanged * pNotifySel, BOOL& bHandled );
	HRESULT OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL& bHandled);
	
	CScnTabConfig();
	~CScnTabConfig();

	virtual void handleTabPreviewChange(TabDataPackage tabData)
	{
		UpdateTabPreview(tabData);
		UpdateSelectedStats(tabData);
	}

};