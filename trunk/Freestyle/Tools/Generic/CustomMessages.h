#pragma once

#define XM_FILES_REFRESH  XM_USER+1

#define XUI_ON_XM_FILES_REFRESH(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_REFRESH)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }


#define XM_FILES_DIRCHANGE  XM_USER+2

#define XUI_ON_XM_FILES_DIRCHANGE(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_DIRCHANGE)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }


#define XM_FILES_FILECHANGE  XM_USER+3

#define XUI_ON_XM_FILES_FILECHANGE(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_FILECHANGE)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }

#define XM_FILES_FILEPRESS  XM_USER+4

#define XUI_ON_XM_FILES_FILEPRESS(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_FILEPRESS)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }

#define XM_FILES_DOBACK  XM_USER+5

#define XUI_ON_XM_FILES_DOBACK(MemberFunc)\
    if (pMessage->dwMessage == XM_FILES_DOBACK)\
    {\
		return MemberFunc(pMessage->bHandled);\
    }

#define XM_SMB_PERMISSION_DENIED  XM_USER+6

#define XUI_ON_XM_SMB_PERMISSION_DENIED(MemberFunc)\
    if (pMessage->dwMessage == XM_SMB_PERMISSION_DENIED)\
    {\
		Credentials *pData = (Credentials *) pMessage->pvData;\
		return MemberFunc(pData, pMessage->bHandled);\
    }

typedef struct {
	string smbPath;
	string user;
	string password;
	string share;
} Credentials;
