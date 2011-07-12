#pragma once

#include "FileOperation.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"

struct FileInformation {
public:
	string path;
	string name;
	string size;
	wstring wname;
	wstring wsize;
	wstring wpath;
	bool isDir;
	bool isSelected;

	string destination;
	bool isCut;
};
	


class FileOperationManager:public CThread
{
private :

	FileOperationManager()
	{
		isDisposed = false;
		m_Paused= true;
		m_currentItem = NULL;
		CreateThread(3);
	};          // Private constructor
  ~FileOperationManager() {
  isDisposed = true;
  }
  FileOperationManager(const FileOperationManager&);                 // Prevent copy-construction
  FileOperationManager& operator=(const FileOperationManager&);      // Prevent assignment
  vector<FileOperation> m_Queue;
  int m_TotalNumberOfFiles;

  ULONGLONG m_TotalSize;
  bool m_Paused;
  ULONGLONG m_SizeDone;
  FileOperation* m_currentItem;
  bool isDisposed;
public :
	ULONGLONG GetTotalSize()
	{
		return m_TotalSize;
	}
	ULONGLONG GetSizeDone()
	{
		ULONGLONG retVal =0;
		retVal= m_SizeDone;
		if(m_currentItem != NULL)
		{
			retVal = retVal + m_currentItem->GetCurrentFileProcess();
		}
		return retVal;
	}
	int GetFilesLeft()
	{
		return m_Queue.size();
	}

	ULONGLONG GetCurrentFileProgress()
	{
		if(m_currentItem != NULL)
		{
		return m_currentItem->GetCurrentFileProcess();
		}
		else
		{
			return 0;
		}

	}

	int GetCurrentProgressPercent()
	{
		if(m_currentItem != NULL)
		{
		return m_currentItem->GetCurrentProgressPercent();
		}
		else
		{
			return 0;
		}
	}

	ULONGLONG GetItemSize()
	{
		if(m_currentItem != NULL)
		{
		return m_currentItem->GetItemSize();
		}
		else
		{
			return 0;
		}
	}

	bool isDone()
	{
		if(m_Queue.size() == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	string GetCurrentItemStatus()
	{
		if(m_currentItem!=NULL)
		{
			return m_currentItem->GetStatus().c_str();
		}
		else
		{
			return "DONE";
		}
	}
	int GetTotalNumberOfFiles()
	{
		return m_TotalNumberOfFiles;
	}
	static FileOperationManager& getInstance()
	{
		static FileOperationManager singleton;
		return singleton;
	}
	void QueueFileOperation(FileOperation op)
	{
		m_Queue.push_back(op);
	}
	void EmptyQueue()
	{
		if(!m_Paused)
		{
			m_Paused = true;
	
	
			if(m_Queue.size() > 0  )
			{
				m_currentItem->Cancel();
	
			}
		}

		m_Queue.clear();
		
	}


	void AddFileOperation(string source, string destination, bool deleteSource);
	void AddFolderOperation(string source, string destination, bool deleteSource);
	void AddVectorOperation(vector<FileInformation> Itm, string destination, bool deleteSource);
	void DoWork()
	{

		m_TotalNumberOfFiles = m_Queue.size();
		m_TotalSize = 0;
		m_SizeDone = 0;
		for(unsigned int x=0;x<m_Queue.size();x++)
		{
			ULONGLONG size = m_Queue.at(x).GetItemSize();
			m_TotalSize = m_TotalSize + size;
		}
		m_Paused = false;
		
	}
	unsigned long Process(void* parameter)
	{
		SetThreadName("FileOperationManager");
		//XSetThreadProcessor(this,5)
	
		while(!isDisposed)
		{
			if(m_Queue.size() > 0  )
			{
				if(!m_Paused)
				{
					XEnableScreenSaver(false);
					
					m_currentItem = &m_Queue.at(0);
				
					m_currentItem->Process();
					m_SizeDone = m_SizeDone + m_currentItem->GetItemSize();
					if(m_Queue.size() != 0) // On cancel queue has been cleared
					{
						m_Queue.erase(m_Queue.begin());
					}
					XEnableScreenSaver(true);
				}
				else
				{
					Sleep(100);
				}
				
			}
			else
			{
				if(!m_Paused)
				{
				
					//Just finished the queue, delete folders?
					m_currentItem = NULL;
					m_Paused = true;
				}
				else
				{
					Sleep(1000);
				}
			}
			Sleep(0);
		}
		return 0;
	}
};