#pragma once
#include "../../../Tools/Generic/tools.h"
#include "../../../Tools/Generic/xboxtools.h"


using namespace std;

class GroupableScene:public CXuiTabSceneImpl
{
private:
	std::map<string,CXuiElement> m_ChildsMap;
	
public :
	GroupableScene();
	//HRESULT GetChildById();
};