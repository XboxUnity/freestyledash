#include "../../../Tools/Debug/Debug.h"

class CScnPluginTrainers : public CXuiTabSceneImpl
{
public:
	CScnPluginTrainers::CScnPluginTrainers() { }
	CScnPluginTrainers::~CScnPluginTrainers() { }

	CXuiList m_TrainerList;
	CXuiScene m_Trainers;
	CXuiTextElement m_TrainerTitle,
					m_TrainerDescrip,
					m_TrainerAuthor;

	bool isNested;

	XUI_IMPLEMENT_CLASS( CScnPluginTrainers, L"ScnPluginTrainers", XUI_CLASS_TABSCENE )

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
	XUI_END_MSG_MAP()

private:
protected:


	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
};