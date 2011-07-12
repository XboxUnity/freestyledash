class CKaiNotifyManager
{
public:
	static CKaiNotifyManager& getInstance()
	{
		static CKaiNotifyManager singleton;
		return singleton;
	}

	HRESULT NotifyXlinkKaiOnline(string aUser);
	HRESULT NotifyXlinkKaiOffline(string aUser);
	HRESULT NotifyContactOnline(string aContact);
	HRESULT NotifyContactOffline(string aContact);
	HRESULT NotifyInviteToGame(string aContact, string aVector);
	HRESULT NotifyReceivedContactPM(string aContact, string aMessage);
	HRESULT NotifyReceivedOpponentPM(string aOpponent, string aMessage);

	HRESULT SendUserNotice(wstring szDisplayString);
	HRESULT UpdateSettings( void );
private:

	struct
	{
		bool NotifyContactOnline;
		bool NotifyContactOffline;
		bool NotifyInviteToGame;
		bool NotifyReceivedContactPM;
		bool NotifyReceivedOpponentPM;
	} NotifySettings;

	CKaiNotifyManager();
	~CKaiNotifyManager() {}

	CKaiNotifyManager(const CKaiNotifyManager&);		            // Prevent copy-construction
	CKaiNotifyManager& operator=(const CKaiNotifyManager&);		    // Prevent assignment
};