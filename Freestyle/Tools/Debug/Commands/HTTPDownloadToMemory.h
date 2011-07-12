#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../HTTP/HTTPDownloader.h"
#include "../../HTTP/HttpItem.h"
#include "../../HTTP/Base/MemoryBuffer.h"

class HTTPDownloadToMemoryCommand:public DebugCommand,public iHttpItemRequester
{
public :
	HTTPDownloadToMemoryCommand()
	{
		m_CommandName = "HTTPDownloadToMemory";
	}
	void Perform(string parameters)
	{
		if( parameters == "")
		{
			parameters = "http://mktplassets.xbox.com/NR/rdonlyres/CBBC3469-BC38-4AD1-9B61-25C553EC3247/0/cbackmasseffect2.jpg";
		}
		HttpItem* itm  = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)parameters.c_str());
		itm->setTag("DebugITEM");
		itm->setRequester(this);
		HTTPDownloader::getInstance().AddToQueue(itm);
	}
	void DownloadCompleted(HttpItem* itm)
	{
	
		DebugMsg("HTTPDownloadToMemory", "Tag:  %s", itm->getTag().c_str());
		DebugMsg("HTTPDownloadToMemory", "Response Code:  %d", itm->getResponseCode());
		DebugMsg("HTTPDownloadToMemory","Download completed");
		MemoryBuffer& buff = itm->GetMemoryBuffer();
		DebugMsg("HTTPDownloadToMemory","Lenght : %d",buff.GetDataLength());
		
	
		string szPath = "hdd1:\\Background.dds";

		ConvertImageInMemoryToDXT5(szPath, buff.GetData(), (DWORD)buff.GetDataLength());
		
		//FILE* f ;
		//fopen_s(&f,("hdd1:\\test.png"),"wb");
		//fwrite(buff.GetData(),(size_t)buff.GetDataLength(),1,f);
		
		//fclose(f);
	}
	
};