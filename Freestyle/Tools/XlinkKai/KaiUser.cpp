#pragma once
#include "stdafx.h"
#include "../Debug/Debug.h"
#include "KaiMail.h"
#include "KaiUser.h"

CKaiUser::CKaiUser( string szUserName, string szPassword, string szLinkedXuid, ULONGLONG ulKaiUserId)
{
	m_szUserName = szUserName;
	m_szPassword = szPassword;
	m_szLinkedXuid = szLinkedXuid;
	m_ulKaiUserId = ulKaiUserId;

	m_szUserNameW = strtowstr(szUserName);
	m_szPasswordW = strtowstr(szPassword);
	m_szLinkedXuidW = strtowstr(szLinkedXuid);

	m_vMessages.clear();
	m_vContacts.clear();
}

CKaiUser::~CKaiUser()
{
	m_vMessages.clear();
	m_vContacts.clear();
	// Deconstructor
}

HRESULT CKaiUser::AddContact(CKaiPlayer * contact)
{
	ContactIterator iter;

	// Return S_FALSE if the contact already exists in the list
	for(iter = m_vContacts.begin(); iter != m_vContacts.end(); ++iter) {
		if(*iter == contact)
			return S_FALSE;
	}

	m_vContacts.push_back(contact);
	return S_OK;
}
HRESULT CKaiUser::RemoveContact(CKaiPlayer * contact)
{
	bool bFoundContact = false;
	ContactIterator iter;

	// Search the vector for a matching contact, when found, erase
	for(iter = m_vContacts.begin(); iter != m_vContacts.end(); ++iter) {
		if(*iter == contact){
			m_vContacts.erase(iter);
			bFoundContact = true;
			break;
		}
	}

	return bFoundContact == true ? S_OK : S_FALSE;
}
HRESULT CKaiUser::AddMessage(CKaiMail * message)
{
	MessageIterator iter;

	// Return S_FALSE if the contact already exists in the list
	for(iter = m_vMessages.begin(); iter != m_vMessages.end(); ++iter) {
		if(*iter == message)
			return S_FALSE;
	}

	m_vMessages.push_back(message);

	return S_OK;
}

HRESULT CKaiUser::RemoveMessage(CKaiMail * message)
{
	bool bFoundMessage = false;
	MessageIterator iter;

	// Search the vector for a matching contact, when found, erase
	for(iter = m_vMessages.begin(); iter != m_vMessages.end(); ++iter) {
		if(*iter == message) {
			m_vMessages.erase(iter);
			bFoundMessage = true;
			break;
		}
	}

	return bFoundMessage == true ? S_OK : S_FALSE;
}