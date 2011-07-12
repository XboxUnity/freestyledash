#pragma once
#include "../../Threads/XeThread.h"
#include "../../GameContent/GameContentManager.h"

typedef struct 
{
	int ListSize;
	int CurrentIndex;
	HXUIBRUSH CurrentBrush;
	bool isRendered;
	bool listLoaded;
	int maxVisible;

} ScreenshotInformation;

class ScreenshotManager
{
public:
	static ScreenshotManager& getInstance()
	{
		static ScreenshotManager singleton;
		return singleton;
	}
	UINT GetScreenshotCount(GameListItem * pGameContent);
	HRESULT CreateScreenshotList(GameListItem * pGameContent);
	HRESULT DestroyScreenshotList();
	HRESULT MoveLeft(int min, int max);
	HRESULT MoveRight(int min, int max);
	HXUIBRUSH GetTextureBrush(int nSSIndex);
	HRESULT SetListInfo(ScreenshotInformation *m_pScreenshotInfo);
	HRESULT GetListInfo(ScreenshotInformation *m_pScreenshotInfo);
	HRESULT Clear();

private:
	// Vector used to store our texture list
	vector<TextureItem *> m_vSSTextures;
	// Vector used to store the file paths 
	vector<string> m_vSSPaths;
	vector<TextureItem *> m_vDisplayList;

	ScreenshotInformation m_ScreenshotList;

	ScreenshotManager()
	{
		m_ScreenshotList.CurrentBrush = NULL;
		m_ScreenshotList.CurrentIndex = 0;
		m_ScreenshotList.ListSize = 0;
		m_ScreenshotList.maxVisible = 0;
		m_ScreenshotList.isRendered = false;
		m_ScreenshotList.listLoaded = false;
	}
	~ScreenshotManager(){}
	
	ScreenshotManager(const ScreenshotManager&);		            // Prevent copy-construction
	ScreenshotManager& operator=(const ScreenshotManager&);		    // Prevent assignment

};