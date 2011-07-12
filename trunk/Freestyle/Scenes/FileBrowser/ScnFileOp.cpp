#include "stdafx.h"

#include "ScnFileOp.h"
#include "../../Tools/Threads/copythread.h"
#include "../../Tools/Debug/Debug.h"
#include "../../Tools/Generic/XboxTools.h"
//#include "scnutilities.h"

string CD_Source;
string CD_Dest;
CD_ActionNames CD_Action;
bool CD_Done = false;

//string CD_DestTitle;

//CCopyThread * ct = NULL;

HRESULT CScnFileOp::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DebugMsg("ScnFileOp","CScnFileOp::OnInit");
	Completed = false;

    GetChildById( L"From", &m_from );
    GetChildById( L"To", &m_to );
	GetChildById( L"Sub1Progress", &m_sub1pb );
	GetChildById( L"Sub2Progress", &m_sub2pb );

	GetChildById( L"FileCount", &m_fileCount );
	GetChildById( L"SizeCompletion", &m_sizeComplete );
	GetChildById( L"PercentComplete", &m_percentComplete );
	GetChildById( L"TransferSpeed", &m_transSpeed );
	GetChildById( L"CurrentFile", &m_curFile );
	GetChildById( L"FileSizeComplete", &m_fileSizeComplete );
	GetChildById( L"FilePercentComplete", &m_filePercentCom );
	GetChildById( L"ETA", &m_ETA );

    GetChildById( L"Stats", &m_stats );
    GetChildById( L"Cancel", &m_cancel );
    GetChildById( L"Title", &m_title );

	btnOk = new LPCWSTR[1]();
	btnOk[0] = L"OK";
	if (IsFolder(CD_Source))
		CD_Source.append("\\");
	CD_Source = str_replaceallA(CD_Source,"\\\\","\\");

	if (IsFolder(CD_Dest))
		CD_Dest.append("\\");
	CD_Dest = str_replaceallA(CD_Dest,"\\\\","\\");

	m_from.SetText(strtowstr("From: " + CD_Source).c_str());
	m_to.SetText(strtowstr("To: " + CD_Dest).c_str());


    CCopyThread::getInstance().Source = CD_Source;
	CCopyThread::getInstance().Dest = CD_Dest;
	CCopyThread::getInstance().Action = CD_Action;
	//CCopyThread::getInstance().Source = "usb0:\\test\\";
	//CCopyThread::getInstance().Dest = "usb0:\\test2\\";

	switch (CD_Action)
	{
	    case CDA_COPYDVD:
		    m_title.SetText(L"Copying DVD to hard drive");
            XamSetDvdSpindleSpeed(DVD_SPEED_12X);
            Sleep(10);
		    break;
	    case CDA_MOVEFILES:
	    case CDA_SDMODE:
		    m_title.SetText(L"Moving files");
		    break;
	    case CDA_COPYFILES:
		    m_title.SetText(L"Copying files");
		    break;
	    case CDA_DELETE:
		    m_title.SetText(L"Deleting files");
		    break;
	}

	BytesLastTick.QuadPart = 0;
	BytesPerSec.QuadPart = 0;

	CCopyThread::getInstance().CreateThread(CPU2_THREAD_2);

	SetTimer(TM_COPYPROGRESS,50);
	SetTimer(TM_COPYSPEED,1000);

	return S_OK;
}

HRESULT CScnFileOp::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
	if (hObjPressed == m_cancel)
	{
		CCopyThread::getInstance().Cancel = true;
		bHandled = TRUE;
	}
	return S_OK;
} 

void CScnFileOp::DisplayResult(string & result)
{
	string action = "Copied";
	if (CD_Action == CDA_MOVEFILES)
		action = "Moved";
	if (CD_Action == CDA_SDMODE)
		action = "Moved";
	if (CD_Action == CDA_DELETE)
		action = "Deleted";

	result.append(sprintfaA("%s:\n%d of %d files\n",action.c_str(),CCopyThread::getInstance().Total_FilesCopied,CCopyThread::getInstance().Total_Files));
	if (CD_Action != CDA_SDMODE || CD_Action == CDA_DELETE)
		result.append(sprintfaA("%0.1fMb of %0.1fMb\n",(float)(CCopyThread::getInstance().Total_Prog.QuadPart+CCopyThread::getInstance().File_Prog.QuadPart)/(1024.0f*1024.0f),(float)(CCopyThread::getInstance().Total_Size.QuadPart)/(1024.0f*1024.0f)));
	result.append("\n\n");
	result.append("Please Note: You will need to restart Freestyle for newly copied games to appear.");
}

HRESULT CScnFileOp::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
    // which timer is it?
    switch( pTimer->nId )
    {
        case TM_COPYPROGRESS:
			{
				if (CCopyThread::getInstance().Stage == CS_CANCEL)
				{
					KillTimer(TM_COPYPROGRESS);
					Completed = true;
					memset(&overlapped,0,sizeof(overlapped));
					string result = "File Copy has been cancelled\n\n";
					DisplayResult(result);
					XShowMessageBoxUI(0,L"Transfer Cancelled",strtowstr(result).c_str(),1,btnOk,0,XMB_ERRORICON,&mbresult,&overlapped);
					SetTimer(TM_OVERLAPPED,50);

				} 
                else if (CCopyThread::getInstance().Stage == CS_DONE)
				{
					KillTimer(TM_COPYPROGRESS);
					Completed = true;
					memset(&overlapped,0,sizeof(overlapped));
					
                  	if (CD_Action != CDA_DELETE)
                    {
                        string result = "File Copy has been completed\n\n";
					    DisplayResult(result);
					    XShowMessageBoxUI(0,L"Transfer Completed",strtowstr(result).c_str(),1,btnOk,0,XMB_NOICON,&mbresult,&overlapped);
					    SetTimer(TM_OVERLAPPED,50);
                    }
				}

				LARGE_INTEGER ETA;
				ETA.QuadPart = 0;
				if (BytesPerSec.QuadPart > 0)
				{
					ETA.QuadPart = (CCopyThread::getInstance().Total_Size.QuadPart - (CCopyThread::getInstance().Total_Prog.QuadPart+CCopyThread::getInstance().File_Prog.QuadPart)) / BytesPerSec.QuadPart;
				}
				int Hours = (int)ETA.QuadPart / 3600;
				ETA.QuadPart = (int)ETA.QuadPart % 3600;
				int Mins = (int)ETA.QuadPart / 60;
				ETA.QuadPart = ETA.QuadPart % 60;
				int Secs = (int)ETA.QuadPart;

				if (CD_Action == CDA_SDMODE || CD_Action == CDA_DELETE)
				{
					m_fileCount.SetText(strtowstr(sprintfaA("%d of %d Files", CCopyThread::getInstance().Total_FilesCopied, CCopyThread::getInstance().Total_Files).c_str()).c_str());
					
					m_sizeComplete.SetShow(false);
					m_transSpeed.SetShow(false);
					m_ETA.SetShow(false);
				} 
                else 
                {
					m_sizeComplete.SetShow(true);
					m_transSpeed.SetShow(true);
					m_ETA.SetShow(true);

					float progress = (float)(CCopyThread::getInstance().Total_Prog.QuadPart+CCopyThread::getInstance().File_Prog.QuadPart)/(1024.0f*1024.0f);
					float size = (float)(CCopyThread::getInstance().Total_Size.QuadPart)/(1024.0f*1024.0f);

					m_fileCount.SetText(strtowstr(sprintfaA("%d of %d Files", CCopyThread::getInstance().Total_FilesCopied, CCopyThread::getInstance().Total_Files).c_str()).c_str());
					m_sizeComplete.SetText(strtowstr(sprintfaA("%0.1fMb of %0.1fMb", progress, size).c_str()).c_str());
					m_transSpeed.SetText(strtowstr(sprintfaA("%0.1fMb/s", (float)BytesPerSec.QuadPart/(1024.0f*1024.0f)).c_str()).c_str());
					m_ETA.SetText(strtowstr(sprintfaA("ETA %d:%02d:%02d", Hours, Mins, Secs).c_str()).c_str());
				}

				if (CCopyThread::getInstance().Stage == CS_CALCSIZE)
				{
					m_curFile.SetText(L"Calculating Size...");
				} 
                else if (CCopyThread::getInstance().Stage == CS_COPYING)
				{
					if (CD_Action == CDA_SDMODE || CD_Action == CDA_DELETE)
					{
						m_curFile.SetText(strtowstr(sprintfaA("%s",CCopyThread::getInstance().GetCurFile().c_str()).c_str()).c_str());

						m_fileSizeComplete.SetShow(false);
					} 
                    else 
                    {
						m_fileSizeComplete.SetShow(true);

						float progress = (float)(CCopyThread::getInstance().File_Prog.QuadPart)/(1024.0f*1024.0f);
						float size = (float)(CCopyThread::getInstance().File_Size.QuadPart)/(1024.0f*1024.0f);

						m_curFile.SetText(strtowstr(sprintfaA("%s",CCopyThread::getInstance().GetCurFile().c_str()).c_str()).c_str());
						m_fileSizeComplete.SetText(strtowstr(sprintfaA("%0.1fMb of %0.1fMb", progress, size).c_str()).c_str());
					}
				} 
                else if (CCopyThread::getInstance().Stage == CS_DONE)
				{
					m_curFile.SetText(L"Done");
				}

                m_sub1pb.SetRange(0,100);
				int totalPer = (int) (((CCopyThread::getInstance().Total_Prog.QuadPart+CCopyThread::getInstance().File_Prog.QuadPart) * 1.0) / (CCopyThread::getInstance().Total_Size.QuadPart * 1.0) * 100.0) + 1;
                m_sub1pb.SetValue(totalPer);
				if(totalPer > -1)
					m_percentComplete.SetText(strtowstr(sprintfaA("%d %%", totalPer).c_str()).c_str());

                m_sub2pb.SetRange(0,100);
				int filePer = (int) ((CCopyThread::getInstance().File_Prog.QuadPart * 1.0) / (CCopyThread::getInstance().File_Size.QuadPart * 1.0) * 100.0) + 1;
                m_sub2pb.SetValue(filePer);
				if(filePer > -1)
					m_filePercentCom.SetText(strtowstr(sprintfaA("%d %%", filePer).c_str()).c_str());
			}
			break;
		case TM_COPYSPEED:
			{
				BytesPerSec.QuadPart = CCopyThread::getInstance().Total_Prog.QuadPart+CCopyThread::getInstance().File_Prog.QuadPart - BytesLastTick.QuadPart;
				BytesLastTick.QuadPart = CCopyThread::getInstance().Total_Prog.QuadPart+CCopyThread::getInstance().File_Prog.QuadPart;
			}
         case TM_OVERLAPPED:
			{
				// message box done
				if (Completed && XHasOverlappedIoCompleted(&overlapped))
				{
					CD_Done = true;
					KillTimer(TM_OVERLAPPED);
					CXuiScene::NavigateBack();
				}
			}
	}
    
    return( S_OK );
}
