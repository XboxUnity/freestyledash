#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../Settings/Settings.h"
#include "../../Texture/TextureItem/TextureItem.h"
#include "../../Texture/TextureCache.h"


class ConvertToDXT5:public DebugCommand
{
public :
	ConvertToDXT5()
	{
		m_CommandName = "ConvertToDXT5";
	}

	void Perform(string parameters)
	{
		string source = parameters;
		vector<string> m_Paths; //Image Paths

		string noTypeName;

		RecursiveFolderSearch(&m_Paths, source, "png");
		RecursiveFolderSearch(&m_Paths, source, "jpg");

		vector<string>::iterator itr;

		for(itr = m_Paths.begin(); itr != m_Paths.end(); itr++)
		{
			DebugMsg("ConvertToDXT5","Converting %s", (*itr).c_str());
			
			//ConvertImageToDXT5(*itr);
		}

		DebugMsg("ConvertToDXT5","Done!");
	}
};