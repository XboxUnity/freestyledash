#include "stdafx.h"
#include "FileCallBack.h"
#include "FileOperationManager.h"
#include "FileCallBackItem.h"
#include "../../Threads/CThread.h"
#include "../../Debug/Debug.h"

FileCallBack::FileCallBack() {

	m_WaitingToStart = FALSE;
	m_CurrentItem = NULL;
	m_IsBusy = FALSE;
	m_IsDisposed = FALSE;
	m_Paused = true;
	cancelTime = false;
	
	CreateThread(CPU3_THREAD_2);
}
FileCallBack::~FileCallBack() {
	m_Queue.clear();
	TimerManager::getInstance().remove(*this);
	m_IsDisposed = TRUE;
}

void	FileCallBack::addSingleOperation(FileCallBackItem* Itm, FileInformation file,string destination, bool isCut) {

	// Wait till we are not busy
	while(IsBusy())
		Sleep(100);
	file.isCut = isCut;
	file.destination = destination;
	m_Queue.push_back(file);
	m_CurrentItem = Itm;
	m_WaitingToStart = TRUE;
	m_Paused = false;
		// Setup our timer
	TimerManager::getInstance().add(*this, 500);
}

void	FileCallBack::addVectorOperation(FileCallBackItem* Itm, vector<FileInformation> file, string destination, bool isCut) {
	
	// Wait till we are not busy
	while(IsBusy())
		Sleep(100);

	for (unsigned int x = 0; x < file.size(); x++)
	{
		file.at(x).isCut = isCut;
		if (destination == "")
			file.at(x).destination = destination;
		else {
			//file.at(x).destination = destination + "\\" + file.at(x).name;

			if (destination.substr(0,4).compare("smb:") == 0) {
				file.at(x).destination = destination + "/" + file.at(x).name;
			}
			else {
				file.at(x).destination = destination + "\\" + file.at(x).name;
			}


		}
		if (file.at(x).isSelected)
			m_Queue.push_back(file.at(x));
	}
	m_CurrentItem = Itm;
	m_WaitingToStart = TRUE;
	m_Paused = false;
		// Setup our timer
	TimerManager::getInstance().add(*this, 500);

}

void	FileCallBack::cancel() {
	TimerManager::getInstance().remove(*this);
	FileOperationManager::getInstance().EmptyQueue();
	m_Queue.clear();
	m_WaitingToStart = FALSE;
	m_CurrentItem = NULL;
	cancelTime = false;
}

void FileCallBack::tick()
{
	if (m_CurrentItem->getIsPrepDone())
	{
		ULONGLONG sizeDone = FileOperationManager::getInstance().GetSizeDone()/1024/1024;
		ULONGLONG totalSize = FileOperationManager::getInstance().GetTotalSize()/1024/1024;
		m_CurrentItem->setSizeDone(sizeDone);
		m_CurrentItem->setSizeTotal(totalSize);
		int nbFilesLeft = FileOperationManager::getInstance().GetFilesLeft();
		int nbFilesTotal = FileOperationManager::getInstance().GetTotalNumberOfFiles();
		int nbFileDone = nbFilesTotal - nbFilesLeft;

		m_CurrentItem->setFilesDone(nbFileDone);
		m_CurrentItem->setTotalFiles(nbFilesTotal);

		m_CurrentItem->setCurrent(FileOperationManager::getInstance().GetCurrentItemStatus());
		m_CurrentItem->setOverallPercent((int)((sizeDone*1.0/totalSize*1.0)*100.0));

		m_CurrentItem->setCurrentSizeComplete(0);
		if(FileOperationManager::getInstance().GetCurrentFileProgress()!= 0)
		{
			m_CurrentItem->setCurrentSizeComplete(FileOperationManager::getInstance().GetCurrentFileProgress()/1024/1024);
		}
		m_CurrentItem->setCurrentSize(FileOperationManager::getInstance().GetItemSize()/1024/1024);

		m_CurrentItem->setCurrentPercent(FileOperationManager::getInstance().GetCurrentProgressPercent());

		m_CurrentItem->setIsDone(FileOperationManager::getInstance().isDone());
		if (m_CurrentItem->requester != NULL)
		{
			m_CurrentItem->requester->UpdateInfo(m_CurrentItem);
		}
		if (m_CurrentItem->getIsDone())
			cancelTime = true;
	}
}

unsigned long FileCallBack::Process(void* parameter) {

	DebugMsg("FileCallBack", "Process");
	
	// Set our name
	SetThreadName("FileCallBack");

	// Now loop while we are not disposed
	while(!m_IsDisposed){

		
		if (cancelTime)
		{
			cancel();
		} else if (m_Queue.size() > 0)
		{
			if (!m_Paused)
			{
				string source;
				if (m_Queue.at(0).path.substr(0,4).compare("smb:") == 0) {
					source = m_Queue.at(0).path + "/" + m_Queue.at(0).name;
				}
				else {
					source = m_Queue.at(0).path + "\\" + m_Queue.at(0).name;
				}

				string filename = m_Queue.at(0).name;
				string destination = m_Queue.at(0).destination;
				bool isCut = m_Queue.at(0).isCut;
				if (m_Queue.at(0).isDir)
				{
					RecursiveMkdir(destination);
					FileOperationManager::getInstance().AddFolderOperation(source, destination, isCut);
				} else {
					FileOperationManager::getInstance().AddFileOperation(source, destination, isCut);
				}
				m_Queue.erase(m_Queue.begin());
			}
		} else {
			if (!m_Paused)
			{
				m_CurrentItem->setPrepDone(true);
				m_CurrentItem->setFirstAfterPrep(true);
				if (m_CurrentItem->requester != NULL)
				{
					m_CurrentItem->requester->UpdateInfo(m_CurrentItem);
				}
				m_Paused = true;
				FileOperationManager::getInstance().DoWork();
			} 
		}
		// Sleep
		Sleep(100);
	}

	// Out of our loop
	return 0;
}