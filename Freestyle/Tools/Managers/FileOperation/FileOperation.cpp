#include "stdafx.h"


#include <sys/stat.h>
#include "../../Generic/tools.h"
#include "../../Debug/Debug.h"
#include "FileOperation.h"
#include "../../../../Libs/libsmbd/xbox/xbLibSmb.h"

DWORD CALLBACK FileManagerCopyProgressRoutine(
  LARGE_INTEGER TotalFileSize, // file size
  LARGE_INTEGER TotalBytesTransferred, // bytes transferred
  LARGE_INTEGER StreamSize, // bytes in stream
  LARGE_INTEGER StreamBytesTransferred, // bytes transferred for stream
  DWORD dwStreamNumber, // current stream
  DWORD dwCallbackReason, // callback reason
  HANDLE hSourceFile, // handle to source file
  HANDLE hDestinationFile, // handle to destination file
  LPVOID lpData // from CopyFileEx
)
{
	FileOperation * fo = (FileOperation*)lpData;
	fo->setCurrentProcess(TotalBytesTransferred.QuadPart);
	if(fo->isCancelled())
	{
		DebugMsg("FileOperation","Cancelling Operation");
		return PROGRESS_CANCEL;
	}
	else
	{
		return PROGRESS_CONTINUE;
	}
}

HRESULT FileOperation::Process()
{
	HRESULT retVal = 0;
	int fid1, fid2;
	FILE* fp1;
	FILE* fp2;
	char *buf;
	int bytes; 
		
    if (strcmp(m_destinationFile.c_str(), "")!=0)
	{
		//Validate destination folder exists
		vector<string> splits;
		DebugMsg("FileOperation","Processing %s",m_destinationFile.c_str());

		if (m_destinationFile.substr(0,4).compare("smb:") == 0) {
			StringSplit(m_destinationFile,"/",&splits);
		}
		else {
			StringSplit(m_destinationFile,"\\",&splits);
		}
		string currentFolder = "";
		for(unsigned int x=0;x<splits.size()-1;x++) //Dont do the file
		{
			string f = splits.at(x);
		
			if(x!=0)
			{
				if (m_destinationFile.substr(0,4).compare("smb:") == 0) {
					if (x ==1) {
						currentFolder = currentFolder + "//";
					}
					else 
						currentFolder = currentFolder + "/";
				}
				else {
					currentFolder = currentFolder + "\\";
				}
			}
			currentFolder = currentFolder + f;
			if(x!=0)
			{
				DebugMsg("FileOperation","Check if %s exist",currentFolder.c_str());
				if (currentFolder.substr(0,4).compare("smb:") == 0) {
					int dir;
					if ((dir = smbc_opendir(currentFolder.c_str())) < 0) {
						DebugMsg("FileBrowser", "could not open dir [%s] (%d:%s)\n", currentFolder.c_str(), errno, strerror(errno));
						smbc_mkdir(currentFolder.c_str(), 0);		
					}	
					smbc_closedir(dir);
				}
				else {
					if(!FileExists(currentFolder.c_str()))
					{
						DebugMsg("FileOperation","Create %s",currentFolder.c_str());
						_mkdir(currentFolder.c_str());
					}
				}
			}
		}
	

		//Move or copy
		DebugMsg("FileOperation","Copy %s to %s",m_sourceFile.c_str(),m_destinationFile.c_str());
		string srcdrive = m_sourceFile.substr(0,m_sourceFile.find(":"));
		string dstdrive = m_destinationFile.substr(0,m_destinationFile.find(":"));
		DebugMsg("FileOperation","Comparing drive %s with %s = %d",srcdrive.c_str(),dstdrive.c_str(),strcmp(srcdrive.c_str(),dstdrive.c_str()));
		if(strcmp(srcdrive.c_str(),dstdrive.c_str())==0 && m_Delete && 
			(m_sourceFile.substr(0,4).compare("smb:") != 0) && 
			(m_destinationFile.substr(0,4).compare("smb:") != 0) ) // If on same drive and cut mode, we can just move 
			// TODO for samba, we can make sure they are on the same share, and use the samba move command...for now, always copy, then delete
		{
			DebugMsg("FileOperation","MoveFile");
			//Same drive, move is instant
			BOOL result = MoveFile(m_sourceFile.c_str(),m_destinationFile.c_str());
			if(!result)
			{
				retVal = -1;
			}
		}	
		else
		{
			if (srcdrive.compare("smb") == 0 || dstdrive.compare("smb") == 0) {
				DebugMsg("FileOperation","Copy to/from smb:");
				if (srcdrive.compare("smb") == 0 && dstdrive.compare("smb") == 0) {
					DebugMsg("FileOperation","Copy to && from smb:");

					if ((fid1 = smbc_open(m_sourceFile.c_str(), O_RDONLY, 0666)) < 0) {				
						 // smbc_open sets errno 
						DebugMsg("FileOperation", "Error, fname=%s, errno=%i", m_sourceFile.c_str(), errno);
						m_Done = true;
						return retVal; 						
					}

					if ((fid2 = smbc_open(m_destinationFile.c_str(), O_WRONLY|O_CREAT, 0666)) < 0) {				
						// smbc_open sets errno 
						DebugMsg("FileOperation", "Error, fname=%s, errno=%i", m_sourceFile.c_str(), errno);
						smbc_close(fid1);
						m_Done = true;
						return -1; 						
					}

					buf = (char*)malloc(4096*4096);
					while ((bytes = smbc_read(fid1, buf, 4096*4096)) > 0) {
						setCurrentProcess(GetCurrentFileProcess() + bytes);

						if ((smbc_write(fid2, buf, bytes)) != bytes) {
							DebugMsg("FileOperation", "Error, didn't write %d bytes", bytes);
							smbc_close(fid1);
							smbc_close(fid2);
						}
					}
					smbc_close(fid1);
					smbc_close(fid2);
					free(buf);

					if(m_Delete)
					{
						_unlink(m_sourceFile.c_str());
					}

				}
				else if (srcdrive.compare("smb") == 0) {
					DebugMsg("FileOperation","Copy from smb:");
					// Try to open the file for reading ... 
					if ((fid1 = smbc_open(m_sourceFile.c_str(), O_RDONLY, 0666)) < 0) {				
						 // smbc_open sets errno 
						DebugMsg("FileOperation", "Error, fname=%s, errno=%i", m_sourceFile.c_str(), errno);
						m_Done = true;
						return retVal; 						
					}

					if ((fp2 = fopen(m_destinationFile.c_str(), "wb")) != NULL) {
						buf = (char*)malloc(4096*4096);
						while ((bytes = smbc_read(fid1, buf, 4096*4096)) > 0) {
							//tot_bytes += bytes;
							setCurrentProcess(GetCurrentFileProcess() + bytes);

							if ((fwrite(buf, 1, bytes, fp2)) != bytes) {
								DebugMsg("FileOperation", "Error, didn't write %d bytes", bytes);
								smbc_close(fid1);
								fclose(fp2);
							}
						}
						smbc_close(fid1);
						fclose(fp2);
						free(buf);
					}
					else {
						DebugMsg("FileOperation", "Couln't fopen %s", m_destinationFile.c_str());
						smbc_close(fid1);
						m_Done = true;
						return retVal;
					}
				}
				else {
					DebugMsg("FileOperation","Copy to smb:");

					if ((fp1 = fopen(m_sourceFile.c_str(), "rb")) != NULL) {

						if ((fid2 = smbc_open(m_destinationFile.c_str(), O_WRONLY|O_CREAT, 0666)) < 0) {				
						 // smbc_open sets errno 
							DebugMsg("FileOperation", "Error, fname=%s, errno=%i", m_sourceFile.c_str(), errno);
							fclose(fp1);
							m_Done = true;
							return -1; 						
						}

						buf = (char*)malloc(4096*4096);
						while ((bytes = fread(buf, 1, 4096*4096, fp1)) > 0) {
							setCurrentProcess(GetCurrentFileProcess() + bytes);
							
							//tot_bytes += bytes;

							if ((smbc_write(fid2, buf, bytes)) != bytes) {
								DebugMsg("FileOperation", "Error, didn't write %d bytes", bytes);
								smbc_close(fid2);
								fclose(fp1);
							}
						}
						smbc_close(fid2);
						fclose(fp1);
						free(buf);
					}
					else {
						DebugMsg("FileOperation", "Couln't fopen %s", m_sourceFile.c_str());
						m_Done = true;
						return -1;
					}
				}

				if(m_Delete) {// was this a move operation?
					if (m_sourceFile.substr(0,4).compare("smb:") == 0) {
						unlinkSamba(m_sourceFile);
					}
				}
			}
			else {  // not dealing with sabma at all
				DebugMsg("FileOperation","CopyFileEx");
				if (CopyFileEx(m_sourceFile.c_str(),m_destinationFile.c_str(),FileManagerCopyProgressRoutine,(void*)this,NULL,0))
				{
					if(m_Delete)
					{
						_unlink(m_sourceFile.c_str());
					}
				}
				else
				{
					retVal = -1;
					if(FileExists(m_destinationFile))
					{
						//Remove file if failed
						_unlink(m_destinationFile.c_str());
					}
				}
			}
		}
	} else {
		if (m_sourceFile.substr(0,4).compare("smb:") == 0) {
			unlinkSamba(m_sourceFile);
		}
		else {
			if (!IsFolder(m_sourceFile))
			{
				//Deleting File
				DebugMsg("FileOperation","DeleteFile %s", m_sourceFile.c_str());
				_unlink(m_sourceFile.c_str());
			} else {
				DebugMsg("FileOperation","DeleteFolder %s", m_sourceFile.c_str());
				RemoveDirectory(m_sourceFile.c_str());
			}
		}
	}
	m_Done = true;
	return retVal;
}

void FileOperation::unlinkSamba(string path) {
	int dir;	
	if ((dir = smbc_opendir(path.c_str())) < 0)
	{
		DebugMsg("FileOperation","unlinkSamba %s", path.c_str());
		smbc_unlink(path.c_str());
	}
	else {
		DebugMsg("FileOperation","unlinkSamba %s", path.c_str());
		smbc_closedir(dir);
		smbc_rmdir(path.c_str());
	}
}
	
