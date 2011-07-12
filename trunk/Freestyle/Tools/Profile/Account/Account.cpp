#include "stdafx.h"
#include "../../Debug/Debug.h"
#include "../../Generic/xboxtools.h"
#include "../../Generic/tools.h"
#include "Account.h"

Account::~Account()
{
	// Ensure that our file is closed before destroying object
	if(fHandle != NULL)
		CloseAccount();
}

HRESULT Account::OpenAccount(string szFileName)
{
	// Open the account file
	fopen_s(&fHandle, szFileName.c_str(), "rb");
	if(fHandle == NULL) return 2;
	
	// Read the encrypted data into a buffer
	char * encryptedBuffer = (char*)malloc(PROFILE_ACCOUNT_ENCRYPTEDSIZE);
	if(encryptedBuffer == NULL) return 3;
	fread(encryptedBuffer, PROFILE_ACCOUNT_ENCRYPTEDSIZE, 1, fHandle);

	// Decrypt Buffer into private XAMACCOUNTINFO struct
	int nDecryptedSize = PROFILE_ACCOUNT_DECRYPTEDSIZE;
	int retVal = XeKeysUnObfuscate(PROFILE_ACCOUNT_OBFUSCATIONBASE, encryptedBuffer, 
		PROFILE_ACCOUNT_ENCRYPTEDSIZE, (char*)&m_sDecryptedAcct, &nDecryptedSize);

	// Release memory
	free(encryptedBuffer);

	return retVal;
}

HRESULT Account::GetXamAccountInfo(PXAMACCOUNTINFO * AccountInfo)
{
	// Point the AccountInfo structure to our decrypted data
	*AccountInfo = &m_sDecryptedAcct;

	return S_OK;
}

HRESULT Account::GetGamerTag(string * szGamerTag)
{
	//*szGamerTag = wchartochar(m_sDecryptedAcct.GamerTag, 0x10);
	return S_OK;
}

HRESULT Account::CloseAccount( void )
{
	// Close handle
	fclose(fHandle);
	return S_OK;
}
