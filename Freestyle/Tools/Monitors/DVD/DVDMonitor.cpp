#include "stdafx.h"
#include "DVDMonitor.h"

#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"


void DVDMonitor::handleDVDNotificationEvent( DWORD dwNotificationID, PULONG_PTR ulParam )
{
	ULONG_PTR ulParams = *ulParam;
	
	switch ( dwNotificationID )
	{
	case XN_SYS_DVDSTATECHANGED:
		if( ulParams == 0 ) // DVD Tray State Changed 
		{
			DVD_TRAY_STATE dvdTrayState = (DVD_TRAY_STATE) XamLoaderGetDvdTrayStateCustom();
			switch (dvdTrayState)
			{
			case TRAY_STATE_EMPTY:
				_NotifyNoDvd();
				break;
			case TRAY_STATE_CLOSING:
				_NotifyTrayClosing();
				break;
			case TRAY_STATE_OPEN:
				_NotifyTrayOpen();
				break;
			case TRAY_STATE_OPENING:
				_NotifyTrayOpening();
				break;
			case TRAY_STATE_CLOSED:
				_NotifyTrayClosed();
				break;
			};
		} 
		else
		{
			switch (ulParams)
			{
			case MEDIA_TYPE_GAME_XBOX_360:
				// Handle Xbox 360 
				_Notify360Dvd();
				break;
			case MEDIA_TYPE_GAME_XBOX_ORIGINAL:
				_NotifyXboxDvd();
				// Handle Xbox Classic
				break;
			case MEDIA_TYPE_CD_AUDIO:
			case MEDIA_TYPE_DVD_AUDIO:
				_NotifyUnknownDvd();
				// Handle Audio CDs
				break;
			case MEDIA_TYPE_DVD_HD:
			case MEDIA_TYPE_DVD_MOVIE:
			case MEDIA_TYPE_CD_VIDEO:
				_NotifyVideoDvd();
				// Handle Video CDs
				break;
			case MEDIA_TYPE_CD_DATA:
				_NotifyDataDvd();
				// Handle Data CD
				break;
			case MEDIA_TYPE_GAME_MOVIE_HYBRID:
			case MEDIA_TYPE_UNKNOWN:
			default:
				// Handle Unrecognized Disc
				_NotifyUnknownDvd();
				break;
			};

		}
	};
}

void DVDMonitor::_NotifyTrayOpen()
{
	DebugMsg("DVDMonitor","Notify Tray Open");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleTrayOpenEvent();
    }
}

void DVDMonitor::_NotifyTrayClosed()
{
	DebugMsg("DVDMonitor","Notify Tray Closed");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleTrayCloseEvent();
    }
	
}
void DVDMonitor::_NotifyTrayOpening()
{
	DebugMsg("DVDMonitor","Notify Tray Opening");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleTrayOpeningEvent();
    }
	
}
void DVDMonitor::_NotifyTrayClosing()
{
	DebugMsg("DVDMonitor","Notify Tray Closing");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleTrayClosingEvent();
    }
	
}
void DVDMonitor::_NotifyDataDvd()
{
	DebugMsg("DVDMonitor","Notify Data DVD");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleDataDVDInsertedEvent();
    }
}

void DVDMonitor::_NotifyUnknownDvd()
{
	DebugMsg("DVDMonitor","Notify 360 DVD");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleUnknownDVDInsertedEvent();
    }
}


void DVDMonitor::_Notify360Dvd()
{
	DebugMsg("DVDMonitor","Notify 360 DVD");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handle360GameDVDInsertedEvent();
    }
}
void DVDMonitor::_NotifyXboxDvd()
{
	DebugMsg("DVDMonitor","Notify XBOX DVD");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleXbox1GameDVDInsertedEvent();
    }
}
void DVDMonitor::_NotifyVideoDvd()
{
	DebugMsg("DVDMonitor","Notify VIDEO DVD");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleVideoDVDInsertedEvent();
    }
}

void DVDMonitor::_NotifyNoDvd()
{
	DebugMsg("DVDMonitor","Notify NO DVD");
	for(DVDitem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
    {
		it->first->handleNoDVDEvent();
    }
}

void DVDMonitor::ForceFirstTrigger()
{
	DebugMsg("DVDMonitor", "Starting Thread");
	bool retVal;
	retVal = CreateThread(CPU3_THREAD_1);
	if(retVal == true)
		DebugMsg("DVDMonitor", "Thread Created");
	else
		DebugMsg("DVDMonitor", "Failed To Create Thread");
}

unsigned long DVDMonitor::Process( void * parameter )
{
	SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	DebugMsg("DVDMonitor", "DVDMonitor Thread Work Begun");
	DoForceTrigger();
	DebugMsg("DVDMonitor", "DVDMonitor Thread Work Complete");

	CloseHandle(this->hThread);
	this->hThread = 0;
	return 0;
}

void DVDMonitor::DoForceTrigger()
{
	DebugMsg("DVDMonitor", "Attempting to force first trigger");

	// First let's force our event handler to detect the tray state so it can update its dependents
	ULONG_PTR ulParam = 0;  // DVD Tray State Change
	handleDVDNotificationEvent( XN_SYS_DVDSTATECHANGED, &ulParam);

	// If the drive is not closed, we can exit- otherwise we need to detect media type
	DVD_TRAY_STATE dvdTrayState = (DVD_TRAY_STATE) XamLoaderGetDvdTrayStateCustom();
	if(dvdTrayState != TRAY_STATE_CLOSED && dvdTrayState != TRAY_STATE_EMPTY)
		return;

	// Now we need to detect the current drive media type
	MEDIA_TYPES mediaType;
	DWORD status = XamLoaderMediaGetInfoCustom(&mediaType);
	if(status == S_FALSE) {
		_NotifyNoDvd();
		return;
	}

	// Now let's send the message with our new content
	ulParam = mediaType;	// Media Type
	handleDVDNotificationEvent( XN_SYS_DVDSTATECHANGED, &ulParam );

	DebugMsg("DVDMonitor", "Force First Trigger Completed Successfully");
}

DVDMonitor::DVDMonitor() 
{
	NotificationMonitor::getInstance().AddObserver(*this);
}   

DVDMonitor::~DVDMonitor()
{
	NotificationMonitor::getInstance().RemoveObserver(*this);
}

void DVDMonitor::add(iDVDObserver& ref)
{
	_observers.insert(DVDitem::value_type(&ref,&ref));
}

void DVDMonitor::remove(iDVDObserver& ref)
{
	_observers.erase(&ref);
}