#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"

class ExtractCONCommand:public DebugCommand
{
public :
	ExtractCONCommand()
	{
		m_CommandName = "ExtractCON";
	}
	void Perform(string parameters)
	{
		DebugMsg("Params :%s",parameters.c_str());
		if(parameters.length() == 0)
		{
			DebugMsg("DebugCommand"," -- ConPath");
			return;
		}
		else
		{
			int firstSlash = parameters.find("\\");
			string drive = parameters.substr(0,firstSlash);
			string path = parameters.substr(firstSlash+1);

			u32 result= mountCon("con",drive.c_str(),path.c_str());
			DebugMsg("DebugCommand","Mount results : %08x",result);
			string filename = "con:\\*.*";
		//FTPMsg("Final \"%s\"",filename.c_str());

		
			WIN32_FIND_DATA findFileData;
			memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
		//FTPMsg(searchcmd.c_str());
			HANDLE hFind = FindFirstFile(filename.c_str(), &findFileData);

		//FTPMsg("LIST %s",filename.c_str());

			if (hFind != INVALID_HANDLE_VALUE)
			{
				do {
					string s = findFileData.cFileName;
					DebugMsg("DebugCommand","DEBUG-GETFILE:%s",s.c_str());

				} while (FindNextFile(hFind, &findFileData));
				FindClose(hFind);
			}
		}
	}
	
};