#include "stdafx.h"

#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Generic/xboxtools.h"
#include "../../../Tools/Debug/Debug.h"
#include "GroupableScene.h"

using namespace std;

GroupableScene::GroupableScene()
{	
	LPCWSTR currentId;
	GetId(&currentId);
	string id = wstrtostr(currentId);
	CXuiElement elem;
	GetFirstChild(&elem);
	while(elem != NULL)
	{
		LPCWSTR cid;
		elem.GetId(&cid);
		DebugMsg("GroupableScene","%s child : %s",id.c_str(),wstrtostr(cid).c_str());
		m_ChildsMap.insert(std::map<string,CXuiElement>::value_type(wstrtostr(cid),elem));
		elem.GetNext(&elem);
	}
}
/*
HRESULT GroupableScene::GetChildById(string id)
{
	return -1;
}*/