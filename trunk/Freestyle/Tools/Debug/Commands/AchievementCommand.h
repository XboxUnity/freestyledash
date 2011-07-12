#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include <xonline.h>

#include "../../FileBrowser/FileBrowser.h"
#include "../../Managers/FileOperation/FileOperationManager.h"

class AchievementCommand:public DebugCommand
{
public :
	AchievementCommand()
	{
		m_CommandName = "ACHV";
	}
	void Perform(string parameters)
	{
		DWORD GameId = 0x535107e4;	 // Final Fantasy XIII
		//DWORD GameId = 0x41560817;   // Call of Duty 4:  Modern Warfare 2

		DWORD bufferSize = 0;
		HANDLE aeHandle = NULL;
		DWORD result = XamUserCreateAchievementEnumerator(
			GameId,
			0,
			INVALID_XUID,
			XACHIEVEMENT_DETAILS_ALL,
			0,
			150,
			&bufferSize,
			&aeHandle
		);

		DWORD returnItems = 0;
		PXACHIEVEMENT_DETAILS pDetails = (PXACHIEVEMENT_DETAILS)malloc(bufferSize);
		result = XEnumerate(
			 aeHandle,
			 pDetails,
			 bufferSize,
			 &returnItems,
			 NULL
		);
		DebugMsg("Achievements", "Items Returned:  %d", returnItems);

		for(int i = 0; i < (int)returnItems; i++)
			DebugMsg("Achv", "Unachieved:  %s", wstrtostr(pDetails[i].pwszUnachieved).c_str());

	/*	IDirect3DTexture9 * pTexture;
		BYTE imageData;
		DWORD imageSize;

		IDirect3DDevice9 * pDevice = RenderLoopManager::getInstance().getD3DDevice();

        HRESULT hr = pDevice->CreateTexture( 64, 64, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, &pTexture, NULL);
		D3DLOCKED_RECT rect;
		D3DSURFACE_DESC desc;

		hr = pTexture->GetLevelDesc( 0, &desc );
		hr = pTexture->LockRect(0, &rect, NULL, 0);

		result = XamReadTileToTexture(0, GameId, pDetails[0].dwImageId, 0, (BYTE*)rect.pBits, rect.Pitch, desc.Height, NULL);
		
		D3DXSaveTextureToFile("hdd1:\\image.png", D3DXIFF_PNG, pTexture, NULL);
		DebugMsg("Achievement", "XamReadTileToTexture Result=%X", result);*/

		free(pDetails);



	}
	
};