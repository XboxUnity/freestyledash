#include "../XEX/Xbox360Container.h"

class Profile : private Xbox360Container
{
public:
	Profile();
	~Profile() {}

	HRESULT OpenProfile(string szProfilePath);
	HRESULT CloseProfile( void );
protected:
private:

	vector<string> m_vGameGPDS;
	vector<string> m_vDashGPDS;
	vector<string> m_vPEC;
	vector<string> m_vAccount;
};