#pragma once

#include "../../SMC/smc.h"
#include "../Notification/NotificationMonitor.h"
#include "../../ContentList/ContentItemNew.h"
#include "../../Threads/CThread.h"

typedef enum _DVD_TRAY_STATE {
	TRAY_STATE_EMPTY,
	TRAY_STATE_CLOSING,
	TRAY_STATE_OPEN,
	TRAY_STATE_OPENING,
	TRAY_STATE_CLOSED
} DVD_TRAY_STATE;

class iDVDObserver{
public :
	virtual void handleNoDVDEvent(){};
	virtual void handleTrayOpenEvent(){};
	virtual void handleTrayOpeningEvent(){};
	virtual void handleTrayCloseEvent(){};
	virtual void handleTrayClosingEvent(){};
	virtual void handleDataDVDInsertedEvent(){};
	virtual void handle360GameDVDInsertedEvent(){};
	virtual void handleXbox1GameDVDInsertedEvent(){};
	virtual void handleVideoDVDInsertedEvent(){};
	virtual void handleUnknownDVDInsertedEvent(){};
};

class DVDMonitor: public CThread, public iNoticeObserver
{
public:
	static DVDMonitor& getInstance()
	{
		static DVDMonitor singleton;
		return singleton;
	}
	void add(iDVDObserver& ref);
	void remove(iDVDObserver& ref);
	
	void ForceFirstTrigger();

	// iNoticeObserver implementation
	void handleDVDNotificationEvent( DWORD dwNotificationID, PULONG_PTR ulParam );

private:	
	std::map<iDVDObserver* const,iDVDObserver* const> _observers;
	typedef std::map<iDVDObserver* const, iDVDObserver* const> DVDitem;

	void DoForceTrigger();
	unsigned long Process( void * parameter );

	void _NotifyTrayOpen();
	void _NotifyTrayClosed();
	void _NotifyTrayOpening();
	void _NotifyTrayClosing();
	void _NotifyDataDvd();
	void _Notify360Dvd();
	void _NotifyXboxDvd();
	void _NotifyVideoDvd();
	void _NotifyUnknownDvd();
	void _NotifyNoDvd();

	DVDMonitor();          // Private constructor
	~DVDMonitor();
	DVDMonitor(const DVDMonitor&);                 // Prevent copy-construction
	DVDMonitor& operator=(const DVDMonitor&);      // Prevent assignment
};
