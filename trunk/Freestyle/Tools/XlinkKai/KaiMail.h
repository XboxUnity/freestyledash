#pragma once
#include "../Generic/xboxtools.h"
#include "../Generic/tools.h"
#include "../Debug/Debug.h"

class CKaiMail
{
private:

	wstring m_szMailMessage;
	wstring m_szMailSender;
	ULONGLONG m_ulMailDateTime;
	ULONGLONG m_ulKaiUserId;
	ULONGLONG m_ulKaiContactId;
	int m_nMailFlag;					// 0 - new, 1 - read, 2 - archived

protected:
public:
	CKaiMail(string strMessage, string strSender, DWORD nFlags, ULONGLONG ulDateTime, ULONGLONG ulKaiUserId, ULONGLONG ulKaiContactId)
	{
		m_szMailMessage = strtowstr(strMessage);
		m_szMailSender = strtowstr(strSender);
		m_ulMailDateTime = ulDateTime;
		m_ulKaiUserId = ulKaiUserId;
		m_ulKaiContactId = ulKaiContactId;
		m_nMailFlag = (int)nFlags;
	}



};