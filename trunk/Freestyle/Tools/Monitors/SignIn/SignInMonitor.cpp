#include "stdafx.h"
#include "../../Debug/Debug.h"
#include "../Notification/NotificationMonitor.h"
#include "SignInMonitor.h"

void SignInMonitor::_notifyPlayerSignedIn(DWORD dwPlayerIndex, XUID newPlayerXuid )
{
	DebugMsg("SingInMonitor", "Notify:  Player %d has Signed In", dwPlayerIndex + 1);
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		it->first->handlePlayerSignedIn(dwPlayerIndex, newPlayerXuid);
	}
}

void SignInMonitor::_notifyPlayerChanged(DWORD dwPlayerIndex, XUID newPlayerXuid, XUID oldPlayerXuid)
{
	DebugMsg("SignInMonitor", "Notify:  Player %d has Changed", dwPlayerIndex + 1);
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		it->first->handlePlayerChanged(dwPlayerIndex, newPlayerXuid, oldPlayerXuid);
	}
}

void SignInMonitor::_notifyPlayerSignedOut(DWORD dwPlayerIndex, XUID oldPlayerXuid)
{
	DebugMsg("SingInMonitor", "Notify:  Player %d has Signed Out", dwPlayerIndex + 1);
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		it->first->handlePlayerSignedOut(dwPlayerIndex, oldPlayerXuid);
	}
}

void SignInMonitor::_notifyStatusChange()
{
	for(item::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		if(&(*it) != NULL)
			it->first->handleStatusChange();
	}
}

SignInMonitor::SignInMonitor() 
{	
	DebugMsg("SignInMonitor","SignIn Monitor Constructor");
	NotificationMonitor::getInstance().AddObserver(*this);
}

SignInMonitor::~SignInMonitor()
{
	DebugMsg("SignInMonitor", "SignIn Monitor Destructor");
	NotificationMonitor::getInstance().RemoveObserver(*this);
}

void SignInMonitor::AddObserver(iSignInObserver& ref)
{
	_observers.insert(item::value_type(&ref, &ref));
}

void SignInMonitor::RemoveObserver(iSignInObserver& ref)
{
	_observers.erase(&ref);
}

void SignInMonitor::handleSignInNotificationEvent(DWORD dwNotificationID, PULONG_PTR ulParam)
{
	ULONG_PTR ulParams = *ulParam;

	// This is here only to provide for future expansion, for now- it is a redundant check
	switch ( dwNotificationID )
	{
	case XN_SYS_SIGNINCHANGED:
		QuerySignInStatus();
		break;
	};
}

void SignInMonitor::QuerySignInStatus( void )
{	
	// Determine the signin state of all 4 potential users
	static XUSER_SIGNIN_STATE pState[PROFILE_MAX_USERS] = { 
		XUserGetSigninState( 0 ), 
		XUserGetSigninState( 1 ),
		XUserGetSigninState( 2 ), 
		XUserGetSigninState( 3 ) 
	};

	// Determine the XUID of all 4 potential users
	XUID tempXuid[PROFILE_MAX_USERS];
	for( int nPlayer = 0; nPlayer < PROFILE_MAX_USERS; nPlayer++ ) 
		XUserGetXUID( nPlayer, &tempXuid[nPlayer] );
	static XUID pXuid[PROFILE_MAX_USERS] = { tempXuid[0], tempXuid[1], tempXuid[2], tempXuid[3] };

	// Broadcast generic status change notificatoin
	_notifyStatusChange();

	// Loop through each player and process the check
	for( int nPlayer = 0; nPlayer < PROFILE_MAX_USERS; nPlayer++ ) {
		// First let's compare the current players xuid to the static xuid
		if( memcmp( &pXuid[nPlayer], &tempXuid[nPlayer], sizeof(XUID)) != 0 ) {
			// Xuid's are different, let's compare our signin state
			XUSER_SIGNIN_STATE tempState = XUserGetSigninState( nPlayer );
			if( pState[nPlayer] != tempState) {
				// The states were different, so let's process the correct callback
				if( tempState == eXUserSigninState_NotSignedIn )
					_notifyPlayerSignedOut( nPlayer, pXuid[nPlayer] );
				else 
					_notifyPlayerSignedIn( nPlayer, tempXuid[nPlayer] );
			} else {
				// The state wasn't changed, but xuid was different, so player changed
				_notifyPlayerChanged( nPlayer, tempXuid[nPlayer], pXuid[nPlayer] );
			}
			// Finished notifying and now we can update our static state and xuid
			pXuid[nPlayer] = tempXuid[nPlayer]; pState[nPlayer] = tempState;
		} // if xuids match
	} // for loop
}