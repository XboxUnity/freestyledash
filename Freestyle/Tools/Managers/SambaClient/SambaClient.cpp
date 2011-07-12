#include "stdafx.h"

#include "SambaClient.h"
#include "../../Debug/Debug.h"
#include "../../SQLite/FSDSql.h"
#include "./Tools/Managers/Drives/DrivesManager.h"
#include "./Tools/Settings/Settings.h"
#include "../../../../Libs/libsmbd/xbox/xbLibSmb.h"

///
//  Get rid of the files that are created in game:?
//  Filter out "self" from the browser?
//  can we dynamically turn on/off the server?
//  careful about the debug level
//  do something about CPU thread assignment IDs
//  BIG: deal with the fact that only a single client can connect...
///

static string smbc_login;
static string smbc_password;

void xb_smbc_log(const char* msg) {
	DebugMsgRaw("libsmb", msg);
}

static void xb_smbc_auth(const char *srv, const char *shr, char *wg, int wglen, char *un, int unlen, char *pw, int pwlen) 
{
	strncpy(un, smbc_login.c_str(), unlen);
	strncpy(pw, smbc_password.c_str(), pwlen);
//	cout << "In xb_smbc_auth for " << srv << " " << shr << " returning: " << smbc_login.c_str() << " [" << smbc_password.c_str() << "]" << endl;
}

void SambaClient::Init() {
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	m_SambaTopLevelTypes.clear();

	///
	/// Initialize smb:
	///
	SMBCCTX *m_context;

	XNADDR addr;
	DWORD retval = XNetGetTitleXnAddr(&addr);

	DebugMsg("SambaClient", "XNetGetTitleXnAddr returned %d", retval);

	char ip[16];
	sprintf_s(ip, 16, "%d.%d.%d.%d", (byte)addr.ina.S_un.S_un_b.s_b1,
		(byte)addr.ina.S_un.S_un_b.s_b2,
		(byte)addr.ina.S_un.S_un_b.s_b3,
		(byte)addr.ina.S_un.S_un_b.s_b4
		);


	set_xbox_interface(ip, "255.255.255.0");   // not required
	set_log_callback(xb_smbc_log);
	SetSambaWorkgroup(SETTINGS::getInstance().getSambaClientWorkgroup());
	m_context = smbc_new_context();
//	smbc_option_set(m_context, "debug_stderr", (void *)1);

	m_context->debug = SAMBA_ERROR_LEVEL;

	//smbc_init(xb_smbc_auth, 0);
	m_context->callbacks.auth_fn = xb_smbc_auth;

//	smbc_option_set(m_context, "auth_function", get_auth_data_with_context_fn);

	//orig_cache = m_context->callbacks.get_cached_srv_fn;
	//m_context->callbacks.get_cached_srv_fn = xb_smbc_cache;
	m_context->options.one_share_per_server = true;
	m_context->options.browse_max_lmb_count = 0;

	m_context->timeout = 5000;

	if (smbc_init_context(m_context))
	{
		smbc_set_context(m_context);
		lp_do_parameter( -1, "name resolve order", "bcast host");
		lp_do_parameter( -1, "dos charset", "CP850");
		smbc_init(xb_smbc_auth, 0);
	}

//	nmbdThread nmbdthread;
//	externmain();


	//browse("smb://", 1, 0);
	///	 DONE with smb: setup

	// Create the Key for the encryption
	string LoadedImageName(GetLoadedImageName());
	string m_ExecutableFilePath = DrivesManager::getInstance().getExecutablePathFromSystemPath(LoadedImageName);
	Drive* ExecutableDrive = DrivesManager::getInstance().getDriveByExecutablePath(m_ExecutableFilePath);
	string ExecutableFolderSystemPath = LoadedImageName.substr(0, LoadedImageName.find_last_of("\\"));
	Drive* dr = new Drive("Game:", ExecutableFolderSystemPath, ExecutableDrive->GetDriveType());
	dr->Mount();
	string key = dr->getSerialStr();
	if (key.length() == 0) {
		key = "aabbccddeeff99887766554433221100";  // completely arbitrary, and bogus, but better than nothing, right?
	}
	oRijndael.MakeKey(key.c_str(), "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16, 16);
	// 

	// if you need to clear something, you can uncomment this
	//FSDSql::getInstance().deleteSambaClientCredential(0, "WORKGROUP/SERVER");

	// if you need to create a credential, you can uncomment the following, and put in the right user and password
	//SetSambaCredentials("WORKGROUP/SERVER", "user", "password", true);
}

void SambaClient::SetSambaWorkgroup(const string& workgroup) {
	xb_setSambaWorkgroup(workgroup.c_str());
}


void SambaClient::SetSambaCredentials(string path, string user, string password, bool bWriteToDB){
	SambaCredentials c;
	c.user = user;
	c.password = password;
	m_SambaCredentials[path] = c;

	if (bWriteToDB) {
		char szHex[33];
		char szDataOut[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
		char szDataIn[17];
		memset(szDataIn, 0, 17);
		strncpy(szDataIn, password.c_str(), 16);  // yes, we only support passwords up to 16 characters long...

		memset(szHex, 0, 33);
		oRijndael.EncryptBlock(szDataIn, szDataOut);
		// turn the 16 byte value into a 32 character hex string
		CharStr2HexStr((unsigned char*)szDataOut, szHex, 16);

		FSDSql::getInstance().setSambaClientCredential(0, path, user, szHex);
	}
}


bool SambaClient::GetSambaCredentials(string path, SambaCredentials& cred) 
{
	string gamerProfileId = GetGamerId(0);

	if (gamerProfile.compare(gamerProfileId) != 0) {
		gamerProfile = gamerProfileId;
		m_SambaCredentials.clear();
		vector<SambaCredentialsEncrypted> v = FSDSql::getInstance().getAllSambaClientCredentials(0);

		for (vector<SambaCredentialsEncrypted>::iterator it = v.begin(); it!=v.end(); ++it) {
			SambaCredentials c;

			char szHex[33];
			char szDataOut[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
			char szDataIn[17];
			memset(szDataIn, 0, 17);
			HexStr2CharStr(((SambaCredentialsEncrypted)*it).encryptedPassword.c_str(), (unsigned char *)&szDataIn, 16);

			oRijndael.DecryptBlock(szDataIn, szDataOut);
			CharStr2HexStr((unsigned char*)szDataOut, szHex, 16);
			char password[17];
			HexStr2CharStr(szHex, (unsigned char *)password, 17);

			c.user = ((SambaCredentialsEncrypted)*it).login;
			c.password = password;  // this is the unencrypted password
			m_SambaCredentials[((SambaCredentialsEncrypted)*it).smbPath] = c;
		}
	}

	std::transform(path.begin(), path.end(), path.begin(), ::toupper);

	map<string, SambaCredentials>::iterator it = m_SambaCredentials.find(path);
	if (it != m_SambaCredentials.end()) {
		SambaCredentials c = it->second;
		cred.password = c.password;
		cred.user = c.user;
		return true;
	}
	return false;
	

	/*
	try {
		SambaCredentialsEncrypted c = FSDSql::getInstance().getSambaClientCredentials(0, path);

		char szHex[33];
		char szDataOut[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
		char szDataIn[17];
		memset(szDataIn, 0, 17);
		HexStr2CharStr(c.encryptedPassword.c_str(), (unsigned char *)&szDataIn, 16);

		oRijndael.DecryptBlock(szDataIn, szDataOut);
		CharStr2HexStr((unsigned char*)szDataOut, szHex, 16);
		char password[17];
		HexStr2CharStr(szHex, (unsigned char *)password, 17);

		cred.password = password;
		cred.user = c.login;
		return true;
	}
	catch (int){
		return false;
	}*/
}

bool SambaClient::TestSambaCredentials(string path, string user, string password) 
{
	int dir;
	smbc_login = user;
	smbc_password = password;
	if ((dir = smbc_opendir(path.c_str())) < 0) {
		smbc_login = "";
		smbc_password = "";
		return false;
	}
	smbc_closedir(dir);
	smbc_login = "";
	smbc_password = "";
	return true;
}

// When passed something like "Move smb://User:Password@foo/bar"
// return "Move smb://foo/bar"
string SambaClient::RemovePassword(string path)
{
	string retval;
	int index = path.find("smb:");
	if (index == string::npos)
		return path;

	int index2 = path.find('@', index);
	if (index2 == string::npos)
		return path;

	retval = path.substr(0,index+6);
	retval += path.substr(index2+1);
	return retval;
}


/*
void SambaClient::handlePlayerChanged( DWORD dwPlayerIndex, XUID newPlayerXuid, XUID oldPlayerXuid )
{
	DebugMsg("SambaClient", "handlePlayerChanged %d %ld %ld", dwPlayerIndex, newPlayerXuid, oldPlayerXuid);
}

void SambaClient::handlePlayerSignedIn( DWORD dwPlayerIndex, XUID newPlayerXuid )
{
	DebugMsg("SambaClient", "handlePlayerSignedIn %d %ld", dwPlayerIndex, newPlayerXuid);
}

void SambaClient::handlePlayerSignedOut( DWORD dwPlayerIndex, XUID oldPlayerXuid )
{
	DebugMsg("SambaClient", "handlePlayerSignedOut %d %ld ", dwPlayerIndex, oldPlayerXuid);
}
*/

static void
get_auth_data_with_context_fn(SMBCCTX * context,
                              const char * pServer,
                              const char * pShare,
                              char * pWorkgroup,
                              int maxLenWorkgroup,
                              char * pUsername,
                              int maxLenUsername,
                              char * pPassword,
							  int maxLenPassword) {
	DebugMsg("ScnDualPane", "Asking for authenication for %s, %s, %s", pServer, pShare, pWorkgroup);
}

// this is just a routine to walk the entire smb: tree...used only for debugging
static void browse(char * path, int scan, int indent)
{
    char *                      p;
    char                        buf[1024];
    int                         dir;
    struct _stat64                 stat;
    struct smbc_dirent *        dirent;

    if (! scan)
    {
        printf("Opening (%s)...\n", path);
    }
        
    if ((dir = smbc_opendir(path)) < 0)
    {
        printf("Could not open directory [%s] (%d:%s)\n",
               path, errno, strerror(errno));
        return;
    }

    while ((dirent = smbc_readdir(dir)) != NULL)
    {
        printf("%*.*s%-30s", indent, indent, "", dirent->name);

        switch(dirent->smbc_type)
        {
        case SMBC_WORKGROUP:
            printf("WORKGROUP");
            break;
            
        case SMBC_SERVER:
            printf("SERVER");
            break;
            
        case SMBC_FILE_SHARE:
            printf("FILE_SHARE");
            break;
            
        case SMBC_PRINTER_SHARE:
            printf("PRINTER_SHARE");
            break;
            
        case SMBC_COMMS_SHARE:
            printf("COMMS_SHARE");
            break;
            
        case SMBC_IPC_SHARE:
            printf("IPC_SHARE");
            break;
            
        case SMBC_DIR:
            printf("DIR");
            break;
            
        case SMBC_FILE:
            printf("FILE");

            p = path + strlen(path);
            strcat(p, "/");
            strcat(p+1, dirent->name);
            if (smbc_stat(path, &stat) < 0)
            {
                printf(" unknown size (reason %d: %s)",
                       errno, strerror(errno));
            }
            else
            {
                printf(" size %lu", (unsigned long) stat.st_size);
            }
            *p = '\0';

            break;
            
        case SMBC_LINK:
            printf("LINK");
            break;
        }

        printf("\n");

        if (scan &&
            (dirent->smbc_type == SMBC_WORKGROUP ||
             dirent->smbc_type == SMBC_SERVER))
        {
            /*
             * don't append server name to workgroup; what we want is:
             *
             *   smb://workgroup_name
             * or
             *   smb://server_name
             *
             */
            sprintf(buf, "smb://%s", dirent->name);
            browse(buf, scan, indent + 2);
        }
    }

    smbc_closedir(dir);
}

