#pragma once


class XZPManager 
{
public:

	static XZPManager& getInstance()
	{
		static XZPManager singleton;
		return singleton;
	}

	HRESULT ExtractXML(string opath, string filename, string xmlname);
	HRESULT XZPOpenBinary(string opath, string filename, string xmlname);
	HRESULT XZPOpenMemory( string xzpPath, string filePath, BYTE ** pFileData, UINT * pFileSize );

private:

	HXUIPACKAGE phPackage;
	unsigned int count;
	WCHAR *szLocator;
	LPWSTR Path;
	string m_basePath;

	HRESULT XZPOpen(string opath, string filename, string xmlname);

	XZPManager(){}
	~XZPManager(){}
	XZPManager(const XZPManager&);                 // Prevent copy-construction
	XZPManager& operator=(const XZPManager&);      // Prevent assignment

};