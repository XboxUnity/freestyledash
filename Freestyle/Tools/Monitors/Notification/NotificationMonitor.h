#pragma once
#include "../../Managers/Timers/TimerManager.h"

// Add Custom/Additional Event Notification IDs Here
#define XN_SYS_DVDSTATECHANGED	0x8000000D


class iNoticeObserver{
public :
	// Add Event handlers Here
	virtual void handleDVDNotificationEvent(DWORD dwNotificationID, PULONG_PTR ulParam){};
	virtual void handleSignInNotificationEvent(DWORD dwNotificationID, PULONG_PTR ulParam){};
	virtual void handleAvatarChangedNotificationEvent(DWORD dwNotificationID, PULONG_PTR ulParam){};
};

class NotificationMonitor : public iTimerObserver
{
public:
	static NotificationMonitor& getInstance()
	{
		static NotificationMonitor singleton;
		return singleton;
	}

	HANDLE m_hNotification;
	void tick();

	void AddObserver(iNoticeObserver& ref);
	void RemoveObserver(iNoticeObserver& ref);

private:
	
	// Add Event Notifiers Here
	void _NotifyDVDEvent(DWORD dwNotificationID, PULONG_PTR ulParam);
	void _NotifySignInEvent(DWORD dwNotificationID, PULONG_PTR ulParam);
	void _NotifyAvatarChangedEvent(DWORD dwNotificationID, PULONG_PTR ulParam);
	
	void CheckForNotifications();

	std::map<iNoticeObserver* const,iNoticeObserver* const> _observers;
	typedef std::map<iNoticeObserver* const, iNoticeObserver* const> item;

	NotificationMonitor();											// Private constructor
	~NotificationMonitor();
	NotificationMonitor(const NotificationMonitor&);                // Prevent copy-construction
	NotificationMonitor& operator=(const NotificationMonitor&);     // Prevent assignment

};
