#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"


class ExtractAvatarDataCommand:public DebugCommand
{
public :
	ExtractAvatarDataCommand()
	{
		m_CommandName = "ExtractAvatarData";
	}
	void Perform(string parameters)
	{
		string szFileName = "";
		char gamertag[100];

		if(strcmp(parameters.c_str(), "") == 0){
			GetGamerTag(0, gamertag);
			szFileName = gamertag;
		}
		else
			szFileName = parameters;

		XAVATAR_METADATA mAvatarMetadata;
		XUID mPlayerXUID;

		XUserGetXUID( 0, &mPlayerXUID);
		DWORD ret = XAvatarGetMetadataLocalUser(0, &mAvatarMetadata, NULL);
		if(ret == ERROR_SUCCESS)
		{
			string szFullPath = "hdd1:\\" + szFileName;
			DebugMsg("ExtractAvatarData", "Dumping Avatar Metadata:  %s", szFullPath.c_str());

			FILE *fHandle;
			fopen_s(&fHandle, szFullPath.c_str(), "wb");
			fwrite(&mAvatarMetadata, 1000, 1, fHandle);
			fclose(fHandle);
		}
	}
	
};