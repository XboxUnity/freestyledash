#pragma once
#include "../Tools/Threads/LoadingThread.h"

class CFreestyleUIApp : public CXuiModule
{

private:
	IXuiDevice * m_pXuiDevice;
public:
	CFreestyleUIApp() {}
	~CFreestyleUIApp() {}
	CFreestyleUIApp(const CFreestyleUIApp&);			// Prevent copy-construction
	CFreestyleUIApp& operator=(const CFreestyleUIApp&); // Prevent assignment

	static CFreestyleUIApp& getInstance()
	{
		static CFreestyleUIApp singleton;
		return singleton;
	}

	IXuiDevice * getXuiDevice() { return m_pXuiDevice; }
    
	virtual void RunFrame();
	HRESULT DestroyMainScene();
	void DispatchXuiInput( XINPUT_KEYSTROKE* pKeystroke );
	HRESULT PreRenderUI( void *pRenderData );
	HRESULT RenderUI(D3DDevice * pDevice, int nHOverscan, int nVOverscan, UINT uWidth, UINT uHeight);
	HRESULT InitializeUI( void * pInitData );
	HRESULT UpdateUI( void * pUpdateData );

	HRESULT CreateMainCanvas();
	HRESULT RegisterXuiClasses();
	HRESULT UnregisterXuiClasses();

	LoadingThread m_LoadingThread;
};