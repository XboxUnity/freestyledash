



class CScnPluginSettings : public CXuiTabSceneImpl
{
public:
	CScnPluginSettings::CScnPluginSettings() { }
	CScnPluginSettings::~CScnPluginSettings() { }

	XUI_IMPLEMENT_CLASS( CScnPluginSettings, L"ScnPluginSettings", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
	XUI_END_MSG_MAP()
private:
protected:



	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
};