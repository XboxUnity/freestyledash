#pragma once

#include <sys/stat.h>
#include "../../Generic/tools.h"
#include "../../Debug/Debug.h"
class FileOperation
{
private :
	string m_sourceFile;
	string m_destinationFile;
	bool m_Delete;
	bool m_Cancel;
	ULONGLONG m_size;
	ULONGLONG m_currentProcess;
	bool m_Done;
public :
	bool isCancelled()
	{
		return m_Cancel;
	}

	void Cancel()
	{
		m_Cancel = true;
		
	}
	
	FileOperation(string source, string dest,bool cut)
	{
		DebugMsg("FileOperation","Add %s to %s",source.c_str(),dest.c_str());
		m_Done = false;
		m_Cancel = false;
		m_Delete = cut;
		m_sourceFile = source;
		m_destinationFile = dest;
		struct stat statFileStatus;
	
		stat(source.c_str(),&statFileStatus);
		
		m_size = statFileStatus.st_size;
		m_currentProcess=0;
	
	} 

	void setCurrentProcess(ULONGLONG currentProcess)
	{
		m_currentProcess = currentProcess;
	}

	string GetStatus()
	{
		string retVal = "";
		if(m_Delete)
		{
			if (strcmp(m_destinationFile.c_str(), "") == 0)
			{
				retVal = "Delete ";
			} else {
				retVal = "Move ";
			}
		}
		else
		{
			retVal = "Copy ";
		}
		retVal = retVal + m_sourceFile;
	
//		string newretVal = sprintfa("%s",retVal,Percent) ;
		return retVal;
	}
		
	ULONGLONG GetItemSize()
	{
		return m_size;
	}
		
	ULONGLONG GetCurrentFileProcess()
	{
		return m_currentProcess;
	}
	
	int GetCurrentProgressPercent()
	{	
		if(m_size != 0)
		{
			ULONGLONG percent = ((m_currentProcess*100)/m_size);
			if (strcmp(m_destinationFile.c_str(), "") == 0)
			{
				percent = 100 - percent;
			}
			return (int)percent;
		} else {
			return 0;
		}
	}

	HRESULT Process();
	void unlinkSamba(string path);		
};