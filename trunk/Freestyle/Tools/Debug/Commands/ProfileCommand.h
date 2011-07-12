#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"

#include "../../Profile/Profile.h"
#include "../../Profile/Account/Account.h"
#include "../../Profile/GPD/GPD.h"


class ProfileCommand:public DebugCommand
{
public :
	ProfileCommand()
	{
		m_CommandName = "Profile";
	}
	void Perform(string parameters)
	{
		if(parameters == "1")
			ForcePlayerSignIn(0, 0xE000158C485D6E42 );
		else if( parameters == "2" )
			ForcePlayerSignIn(0, 0xE000158C485D6E42 );

		return;


		vector<string> args;
		StringSplit(parameters, " ", &args);
		
		//if(args.size() != 2) return;
		DebugMsg("ProfileCommand", "Action:  '%s',  Parameters:  '%s'", args.at(0).c_str(), args.at(1).c_str());
	
		
		if(strcmp(args.at(0).c_str(), "account") == 0)
			ExtractAccount(args.at(1).c_str());

		if(strcmp(args.at(0).c_str(), "gpd") == 0)
			EnumerateGPD(args.at(1).c_str(), args.at(2).c_str());

		if(strcmp(args.at(0).c_str(), "profiles") == 0)
			EnumerateProfiles(parameters);
	}

	void EnumerateProfiles(string parameters)
	{
		HANDLE pHandle = NULL;
		DWORD result = XamProfileCreateEnumerator(
			0, // Flags?? 0 = All, 1 = XUID Only
			&pHandle
		);

		XUID xuids[2];
		int i = 0;
		PPROFILEENUMRESULT pProfile = (PPROFILEENUMRESULT)malloc(sizeof(PROFILEENUMRESULT));
		while(result == S_OK){

			ZeroMemory(pProfile, sizeof(PROFILEENUMRESULT));
			result = XamProfileEnumerate(
				pHandle, 0, pProfile, NULL
			);
			if(result == S_OK)
			{
				xuids[i] = pProfile->xuidOffline;
				DebugMsg("Profile", "Enum Result:  %I64X, %s", pProfile->xuidOffline, wchartochar(pProfile->xai.szGamerTag, 16));
				i = i + 1;
			}
		}
		//Enum Result:  E000042C485D6E42, Rowen
		//ForcePlayerSignOut(0);
		result = ForcePlayerSignIn(0, 0xE000158C485D6E42); // Rowen
		//result = ForcePlayerSignIn(0, 0xE000035F485D6E42); // MaesterRowen
		//result = ForcePlayerSignOut(3);
//		DebugMsg("Profile" ,"ForcePlayerSignIn Result:  %X", result);
		
		//XUID myxuids[4] = {INVALID_XUID ,  INVALID_XUID, INVALID_XUID, INVALID_XUID };
	//	result = XamUserLogon(myxuids, 0x23, NULL);			
		free(pProfile);
		//XUID xuids[4] = {INVALID_XUID, INVALID_XUID, INVALID_XUID, INVALID_XUID};
		//xuids[0] = pProfile->xuidOffline;
		//result = XamUserLogon(xuids, 0x23/*0x424*/, NULL);

		

	}

	void EnumerateGPD( string fileIn, string fileOut )
	{
		GPD gpdFile;
		DebugMsg("GPD", "Enumerate GPD Start");

		gpdFile.OpenGPD(fileIn);

		int nEntries = gpdFile.Header.EntryCurrent;
		DebugMsg("GPD", "Entry Count:  %d", nEntries);

		for(int i = 0; i < nEntries; i++)
		{
			DebugMsg("GPD", "ID:  %I64X,  NameSpace:  %X,  Offset:  %X,  Size:  %X", gpdFile.EntryTable[i].Id, gpdFile.EntryTable[i].NameSpace, gpdFile.EntryTable[i].Offset, gpdFile.EntryTable[i].Size);
		}

		FILE * fHandle;
		BYTE * fileData;
		int fileSize;

		gpdFile.GetSection(3, 0x63E80044, &fileData, &fileSize);
		fopen_s(&fHandle, fileOut.c_str(), "wb");
		fwrite(fileData, fileSize, 1, fHandle);
		fclose(fHandle);

		DebugMsg("GPD", "%s - written successfully", fileOut.c_str());

		gpdFile.CloseGPD();

	}

	void ExtractAccount(string parameters)
	{
/*		DebugMsg("ExtractAccount", "Parameter Received:  '%s'", parameters.c_str());
		Account acct;

		acct.OpenAccount("hdd1:\\Account");

		PXAMACCOUNTINFO acctInfo;
		acct.GetXamAccountInfo(&acctInfo);
		string gamerTag = wchartochar(acctInfo->GamerTag, 0x10);
		DebugMsg("Test", "Test:  %s", gamerTag.c_str());

		DebugMsg("test", "sizeof(UINT) = %X", sizeof(UINT));

		DebugMsg("Test", "Test:  %X", acctInfo->Passcode);
		//acctInfo->
	

*/

		//acct.CloseAccount();
	}	
};