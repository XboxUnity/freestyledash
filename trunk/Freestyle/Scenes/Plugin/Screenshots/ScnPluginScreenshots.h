class CScnPluginScreenshots : public CXuiTabSceneImpl
{
public:
	CScnPluginScreenshots::CScnPluginScreenshots() { }
	CScnPluginScreenshots::~CScnPluginScreenshots() { }

	XUI_IMPLEMENT_CLASS( CScnPluginScreenshots, L"ScnPluginScreenshots", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
	XUI_END_MSG_MAP()

private:
protected:

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
};