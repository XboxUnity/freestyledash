#pragma once
#include "../Notification/NotificationMonitor.h"

#define PROFILE_MAX_USERS	4

class iSignInObserver{
public :
	virtual void handlePlayerSignedIn( DWORD dwPlayerIndex, XUID newPlayerXuid ){};
	virtual void handlePlayerChanged( DWORD dwPlayerIndex, XUID newPlayerXuid, XUID oldPlayerXuid ){};
	virtual void handlePlayerSignedOut( DWORD dwPlayerIndex, XUID oldPlayerXuid ){};
	virtual void handleStatusChange(){};
};

class SignInMonitor: public iNoticeObserver
{
public:
	static SignInMonitor& getInstance()
	{
		static SignInMonitor singleton;
		return singleton;
	}

	void AddObserver(iSignInObserver& ref);
	void RemoveObserver(iSignInObserver& ref);
	
// Other non-static member functions
private:	   
	void QuerySignInStatus( void );

	std::map<iSignInObserver* const,iSignInObserver* const> _observers;
	typedef std::map<iSignInObserver* const, iSignInObserver* const> item;
	
	void _notifyPlayerSignedIn( DWORD dwPlayerIndex, XUID newPlayerXuid );
	void _notifyPlayerChanged( DWORD dwPlayerIndex, XUID newPlayerXuid, XUID oldPlayerXuid );
	void _notifyPlayerSignedOut( DWORD dwPlayerIndex, XUID oldPlayerXuid  );
	void _notifyStatusChange( );

	virtual void handleSignInNotificationEvent(DWORD dwNotificationID, PULONG_PTR ulParam);

	SignInMonitor();									   // Private constructor
	~SignInMonitor();
	SignInMonitor(const SignInMonitor&);                 // Prevent copy-construction
	SignInMonitor& operator=(const SignInMonitor&);      // Prevent assignment
};
