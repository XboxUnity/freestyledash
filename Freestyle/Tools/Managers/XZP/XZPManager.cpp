#include "stdafx.h"

#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
#include "../../Settings/Settings.h"
#include "XZPManager.h"

HRESULT XZPManager::ExtractXML(std::string opath, std::string filename, std::string xmlname)
{
	HRESULT hr = XZPOpen(opath, filename, xmlname);
	return hr;
}

HRESULT XZPManager::XZPOpen(string opath, string filename, string xmlname)
{
	m_basePath = SETTINGS::getInstance().getDataPath();
	DebugMsg("DEBUG", "%s", m_basePath.c_str());
	string loadfilename = "file://" + opath + filename;
	loadfilename = str_replaceall(loadfilename, "\\", "/");
	Path = strtowchar(loadfilename);
	DebugMsg("XZPManager", "PATH: %s, loading package", wstrtostr(Path).c_str());
	HRESULT hr = XuiResourceOpenPackage(Path,&phPackage,false);
	if (hr != S_OK)
	{
		DebugMsg("XZPManager", "Error HRESULT: %08x", hr);
		return hr;
	} else {
		szLocator = strtowchar(loadfilename + "#" + xmlname);

		byte * buf;
		UINT size;
		hr = XuiResourceLoadAllNoLoc(szLocator, &buf, &size);
		if (hr != S_OK)
		{
			DebugMsg("XZPManager", "HRESULT : %08x, Loading resource");
			XuiResourceReleasePackage(phPackage);
			return hr;
		} else {
			DebugMsg("XZPManager", "Resource is in memory Size is %d", size);
			string out = (char*)buf;
			string outpath = sprintfaA("%sSkins\\%s\\%s", m_basePath.c_str(), filename.c_str(), xmlname.c_str());
			DebugMsg("XZPManager", "Saving file to %s", outpath.c_str());
			StringToFile(out, outpath);
			XuiFree((void*)buf);
		}
	}

	XuiResourceReleasePackage(phPackage);
	return S_OK;
}

HRESULT XZPManager::XZPOpenBinary(string opath, string filename, string xmlname)
{
	m_basePath = SETTINGS::getInstance().getDataPath();
	string loadfilename = "file://" + opath + filename;
	loadfilename = str_replaceall(loadfilename, "\\", "/");
	Path = strtowchar(loadfilename);
	DebugMsg("XZPManager", "PATH: %s, loading package", wstrtostr(Path).c_str());
	HRESULT hr = XuiResourceOpenPackage(Path,&phPackage,false);
	if (hr != S_OK)
	{
		DebugMsg("XZPManager", "Error HRESULT: %08x", hr);
		return hr;
	} else {
		szLocator = strtowchar(loadfilename + "#" + xmlname);

		byte * buf;
		UINT size;
		hr = XuiResourceLoadAllNoLoc(szLocator, &buf, &size);
		if (hr != S_OK)
		{
			DebugMsg("XZPManager", "HRESULT : %08x, Loading resource");
			XuiResourceReleasePackage(phPackage);
			return hr;
		} else {
			DebugMsg("XZPManager", "Resource is in memory Size is %d", size);
			string outpath = sprintfaA("%sSkins\\%s\\%s", m_basePath.c_str(), filename.c_str(), xmlname.c_str());
			DebugMsg("XZPManager", "Saving file to %s", outpath.c_str());

			// Write Item To File
			FILE * fHandle;
			fopen_s(&fHandle, outpath.c_str(), "wb");
			fwrite(buf, size, 1, fHandle);
			fclose(fHandle);

			XuiFree((void*)buf);
		}
	}

	XuiResourceReleasePackage(phPackage);
	return S_OK;
}

HRESULT XZPManager::XZPOpenMemory( string xzpPath, string filePath, BYTE ** pFileData, UINT * pFileSize )
{
	HRESULT hr = NULL;
	HXUIPACKAGE hPackage;

	if(!FileExistsA(xzpPath))
	{
		DebugMsg("XZPManager", "Provided XZP File Path Does Not Exist");
		return S_FALSE;
	}

	
	// Convert absolute path to xui path
	string xuiPath = "file://" + xzpPath;
	
	hr = XuiResourceOpenPackage(strtowchar(xuiPath), &hPackage, false);
	if(hr != S_OK)
	{
		DebugMsg("XZPManager", "Failed To Open XZP Package");
		return S_FALSE;
	}

	WCHAR * szLocator = strtowchar(xuiPath + "#" + filePath);
	
	BYTE * pSectionData;
	UINT pSectionSize;

	hr = XuiResourceLoadAllNoLoc(szLocator, &pSectionData, &pSectionSize);
	if( hr != S_OK )
	{
		DebugMsg("XZPManager", "Failed To Load Resource File.");
		return S_FALSE;
	}

	*pFileData = (BYTE *)malloc(pSectionSize);
	memcpy(*pFileData, pSectionData, pSectionSize);
	*pFileSize = pSectionSize;
	XuiFree((void*)pSectionData);

	XuiResourceReleasePackage(hPackage);
	

	return S_OK;
}