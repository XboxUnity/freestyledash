#pragma once
#include "stdafx.h"
#include "ContentKaiVector.h"
#include "../HTTP/HTTPDownloader.h"
#include "../HTTP/HttpItem.h"
#include "./KaiVectorXML/KaiVectorXML.h"
#include "../Settings/Settings.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ContentKaiVector::ContentKaiVector()
{
  InitializeCriticalSection(&m_critical);
  m_bDirty = false;
  m_nVersion = 0;

  vectorDataPath = GetBasePath();
}

ContentKaiVector::~ContentKaiVector()
{
  DeleteCriticalSection(&m_critical);
}

string ContentKaiVector::GetBasePath()
{
	// Get our game data path
	string dataPath = SETTINGS::getInstance().getDataPath();
	dataPath = dataPath + "\\XlinkKaiData";
	dataPath = str_replaceallA(dataPath, "\\\\", "\\");

	if(!FileExistsA(dataPath))
		_mkdir(dataPath.c_str());

	return dataPath;
}

void ContentKaiVector::Save(const string strPath)
{
	if (m_bDirty)
	{
		// Remove old file	
		_unlink(strPath.c_str());
		EnterCriticalSection(&m_critical);

		string strXMLText;
		strXMLText = sprintfaA("<version>%d</version>\n", m_nVersion);
		strXMLText.append("<titles>\n");
		for (TITLEVECTORMAP::iterator it = m_mapTitles.begin(); it != m_mapTitles.end(); it++)
		{			
			string strTitleId, strVector;
			DWORD dwTitleId = it->first;
			strTitleId = sprintfaA("0x%X", dwTitleId);
			strVector = it->second;	
			
			strXMLText.append("\t<title>\n");
			strXMLText.append("\t\t<id>");
			strXMLText.append(strTitleId);
			strXMLText.append("</id>\n");
			strXMLText.append("\t\t<vector>");
			strXMLText.append(strVector);
			strXMLText.append("</vector>\n");
			strXMLText.append("\t</title>\n");
		}

		strXMLText.append("</titles>");
		LeaveCriticalSection(&m_critical);

		StringToFile(strXMLText, strPath);
		DebugMsg("ContentKaiVector", "File: %s saved completed.", strPath.c_str());
		m_bDirty = false;
	  }
}

void ContentKaiVector::Initialize( void ) {
	//string fullPath = vectorDataPath + "\\KaiContent.bin";
	//Load(fullPath);
	DownloadAppSpecVersion(VERSION_FILEURL);
}

void ContentKaiVector::Load(const string strPath)
{
	if(FileExists(strPath))
	{
		m_mapTitles.clear();

		KaiVectorXML kaiXml;
		kaiXml.SetTitleMap(&m_mapTitles);
		kaiXml.SetVectorMap(&m_mapVectors);
		ATG::XMLParser parser;
		parser.RegisterSAXCallbackInterface(&kaiXml);
		parser.ParseXMLFile(strPath.c_str());

		
		int nSize = m_mapTitles.size();
		std::map<DWORD, string>::iterator itr;
		//for( itr = m_mapTitles.begin(); itr != m_mapTitles.end(); ++itr)
		//{

		//	DebugMsg("CONTENTKAIVECTOR", "ITEMS FOUND:  %s", (itr)->second.c_str());
		//}

		DebugMsg("ContentKaiVector", "Loaded %d entries", nSize);

		if(m_nVersion > kaiXml.GetVersion())
			DownloadAppSpec(APPSPEC_FILEURL);
	}
	else
	{
		DownloadAppSpec(APPSPEC_FILEURL);
	}
}



void ContentKaiVector::AddTitle(DWORD aTitleId, string& aVector)
{
  EnterCriticalSection(&m_critical);
  m_mapTitles[aTitleId] = aVector;
  m_mapVectors[aVector] = aTitleId;
  LeaveCriticalSection(&m_critical);
  m_bDirty = true;
}

bool ContentKaiVector::GetVectorByTitleId(DWORD aTitleId, string& aVector)
{
  EnterCriticalSection(&m_critical);

  TITLEVECTORMAP::iterator it = m_mapTitles.find(aTitleId);
  bool bContainsTitle = ( it != m_mapTitles.end() );
  if (bContainsTitle)
  {
    aVector = m_mapTitles[aTitleId];
  }

  LeaveCriticalSection(&m_critical);
  return bContainsTitle;
}

bool ContentKaiVector::GetTitleIdByVector(string& aVector, DWORD * aTitleId )
{
  EnterCriticalSection(&m_critical);

  VECTORTITLEMAP::iterator it = m_mapVectors.find(aVector);
  bool bContainsTitle = ( it != m_mapVectors.end() );
  if (bContainsTitle)
  {
	*aTitleId = m_mapVectors[aVector];
  }

  LeaveCriticalSection(&m_critical);
  return bContainsTitle;

}
bool ContentKaiVector::ContainsTitle(DWORD aTitleId)
{
  EnterCriticalSection(&m_critical);

  bool bContainsTitle = ( m_mapTitles.find(aTitleId) != m_mapTitles.end() );

  LeaveCriticalSection(&m_critical);
  return bContainsTitle;
}

bool ContentKaiVector::IsEmpty()
{
  EnterCriticalSection(&m_critical);

  bool bIsEmpty = ( m_mapTitles.begin() == m_mapTitles.end() );

  LeaveCriticalSection(&m_critical);
  return bIsEmpty;
}

void ContentKaiVector::DownloadAppSpec(string strUrl)
{
	HttpItem * item = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)strUrl.c_str());
	item->SetPriority(HTTP_PRIORITY_HIGH);
	item->setTag(APPSPEC_URLTAG);
	item->setRequester(this);	
	HTTPDownloader::getInstance().AddToQueue(item);
}

void ContentKaiVector::DownloadAppSpecVersion(string strUrl)
{
	HttpItem * item = HTTPDownloader::getInstance().CreateHTTPItem((CHAR*)strUrl.c_str());
	item->SetPriority(HTTP_PRIORITY_HIGH);
	item->setTag(VERSION_URLTAG);
	item->setRequester(this);
	HTTPDownloader::getInstance().AddToQueue(item);
}

void ContentKaiVector::DownloadCompleted(HttpItem * itm)
{
	if(itm->getResponseCode() == 200)
	{
		if(strcmp(itm->getTag().c_str(), VERSION_URLTAG) == 0)
		{
			MemoryBuffer& buff = itm->GetMemoryBuffer();
			DWORD dwLen = buff.GetDataLength();
			BYTE * verData = buff.GetData();
			CHAR * context = NULL;
			CHAR * szData = strtok_s( (char*)verData, ";", &context);

			if(szData != NULL && strcmp(szData, VERSION_HEADER) == 0) {
				m_nVersion = atoi(strtok_s(NULL, ";", &context));
				DebugMsg("ContentKaiVector", "Vector Version = %d", m_nVersion);
			}

			context = NULL;

			string fullPath = vectorDataPath + "\\KaiContent.bin";
			Load(fullPath);
		}
		else if(strcmp(itm->getTag().c_str(), APPSPEC_URLTAG) == 0)
		{
			m_mapTitles.clear();
			MemoryBuffer& buff = itm->GetMemoryBuffer();
			buff.GetDataLength();
			BYTE * phpData = buff.GetData();
			CHAR * context = NULL;

			
			CHAR * szData = strtok_s( (char*)phpData, ";", &context);
			string szHeader = szData;
			if(szData != NULL && szHeader.find(APPSPEC_HEADER) != string::npos)
			{

				string strXex, strArena;
				CHAR * szToken;
			
				while( (szToken = strtok_s( NULL, ";", &context )) != NULL)
				{
					strXex = szToken;
					if(strXex.find(APPSPEC_PREFIX) == 0 || strXex.find(APPSPEC_PREFIX_CLASSIC) == 0)
					{
						strArena = strtok_s(NULL, ";", &context);
						DWORD dwTitleId = strtoul(strXex.substr(4, strXex.length()).c_str(), NULL, 16);
						strArena = str_replaceallA(strArena, "&", "&amp;");
						AddTitle(dwTitleId, strArena);
					}
				}
				m_bDirty = true;
				context = NULL;

				DebugMsg("ContentKaiVector", "Download and Parsing Completed Successfully.");

				Save(vectorDataPath + "\\KaiContent.bin");
			}		
		}
	}
	else
	{
		if(strcmp(itm->getTag().c_str(), VERSION_URLTAG) == 0)
		{
			DebugMsg("ContentKaiVector", "Failed to Download Version File, Processing existing data");
			// HttpItem failed- we'll need to try to load the item already from file, or fail completely
			string fullPath = vectorDataPath + "\\KaiContent.bin";
			Load(fullPath);	
		}
	}
}


