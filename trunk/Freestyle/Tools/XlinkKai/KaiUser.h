#pragma once
#include "../Generic/xboxtools.h"
#include "KaiMail.h"
#include "KaiPlayer.h"

class CKaiUser
{
private:
	string m_szUserName;
	string m_szPassword;
	string m_szLinkedXuid;
	ULONGLONG m_ulKaiUserId;

	wstring m_szUserNameW;
	wstring m_szPasswordW;
	wstring m_szLinkedXuidW;

	vector<CKaiMail*> m_vMessages;
	vector<CKaiPlayer*> m_vContacts;

	typedef vector<CKaiMail*>::iterator MessageIterator;
	typedef vector<CKaiPlayer*>::iterator ContactIterator;
public:

	CKaiUser( string szUserName, string szPassword, string szLinkedXuid, ULONGLONG ulKaiUserId );
	~CKaiUser();

	HRESULT AddContact(CKaiPlayer * contact);
	HRESULT RemoveContact(CKaiPlayer * contact);
	HRESULT AddMessage(CKaiMail * message);
	HRESULT RemoveMessage(CKaiMail * message);

	void SetUserName( string szUserName )		{ m_szUserName = szUserName; }
	void SetPassword( string szPassword )		{ m_szPassword = szPassword; }
	void SetLinkedXuid( string szLinkedXuid )	{ m_szLinkedXuid = szLinkedXuid; }
	void SetKaiUserId( ULONGLONG ulKaiUserId )	{ m_ulKaiUserId = ulKaiUserId; }

	string getUserName( void )					{ return m_szUserName; }
	string getPassword( void )					{ return m_szPassword; }
	string getLinkedXuid( void )				{ return m_szLinkedXuid; }
	ULONGLONG getKaiUserId( void )				{ return m_ulKaiUserId; }

	// Overloaded for use with XUI
	LPCWSTR getUserNameW( void )				{ return m_szUserNameW.c_str(); }
	LPCWSTR getPasswordW( void )				{ return m_szPasswordW.c_str(); }
	LPCWSTR getLinkedXuidW( void )				{ return m_szLinkedXuidW.c_str(); }

	CKaiMail * getMessage( int nMessageIndex )  { return m_vMessages.at(nMessageIndex); }
	CKaiPlayer * getContact( int nContactIndex ){ return m_vContacts.at(nContactIndex); }
};