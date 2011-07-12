#pragma once
#include "../../Generic/xboxtools.h"

#define PROFILE_ACCOUNT_ENCRYPTEDSIZE	0x194
#define PROFILE_ACCOUNT_DECRYPTEDSIZE	0x17C
#define PROFILE_ACCOUNT_OBFUSCATIONBASE 0x1



class Account
{
public:
	Account() {}
	~Account();

	HRESULT OpenAccount(string szFileName);
	HRESULT GetXamAccountInfo(PXAMACCOUNTINFO * AccountInfo);
	HRESULT GetGamerTag(string * szGamerTag);
	//HRESULT EncryptAccount( void );
	HRESULT CloseAccount( void );
private:
	FILE * fHandle;
	XAMACCOUNTINFO m_sDecryptedAcct;


};