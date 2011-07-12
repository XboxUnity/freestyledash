#pragma once
#include "../../Monitors/SignIn/SignInMonitor.h"
#include "../../AES/Rijndael.h"

using namespace std;

struct SambaCredentials {
	string user;
	string password;
};

class SambaClient //: public iSignInObserver
{
public:
	static SambaClient& getInstance()
	{
		static SambaClient singleton;
		return singleton;
	}

	 void Init();

	map<string, unsigned int> m_SambaTopLevelTypes;

	bool GetSambaCredentials(string path, SambaCredentials& cred);
	void SetSambaCredentials(string path, string user, string password, bool bWriteToDB);
	bool TestSambaCredentials(string path, string user, string password);
	void SetSambaWorkgroup(const string& workgroup);

	string RemovePassword(string path);
	/*virtual void handlePlayerSignedIn( DWORD dwPlayerIndex, XUID newPlayerXuid );
	virtual void handlePlayerChanged( DWORD dwPlayerIndex, XUID newPlayerXuid, XUID oldPlayerXuid );
	virtual void handlePlayerSignedOut( DWORD dwPlayerIndex, XUID oldPlayerXuid );*/


private:
	SambaClient() : gamerProfile("x") {
		//SignInMonitor::getInstance().AddObserver(*this);
		Init();
	}
	~SambaClient() {};
	SambaClient(const SambaClient&);                 // Prevent copy-construction
	SambaClient& operator=(const SambaClient&);      // Prevent assignment

	CRijndael oRijndael;

	string gamerProfile;
	map<string, SambaCredentials> m_SambaCredentials;
};


