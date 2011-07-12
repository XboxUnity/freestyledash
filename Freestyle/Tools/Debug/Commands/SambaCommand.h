#include "stdafx.h"
#include "../../Generic/tools.h"
#include "../../Generic/xboxtools.h"
#include "../Debug.h"
#include "DebugCommand.h"
#include "../../../../Libs/libsmbd/xbox/xbLibSmb.h"

class SambaCommand:public DebugCommand
{
public :
	SambaCommand()
	{
		m_CommandName = "Samba";
	}

	void Perform(string parameters)
	{
		SMBCCTX *m_context;

		set_xbox_interface("192.168.0.147", "255.255.255.0");
		set_log_callback(xb_smbc_log);
		xb_setSambaWorkgroup("workgroup");
		m_context = smbc_new_context();
		m_context->debug = 10;
		smbc_init(xb_smbc_auth, 0);
		m_context->callbacks.auth_fn = xb_smbc_auth;
		//need to fix these lines
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
		}

		//get file size
		struct __stat64 info;
		//need to format "" to be a samba url with authentication
		int iResult = smbc_stat("smb://server/public/xm360/xm360.xex", &info);

		smbc_set_context(NULL);
		smbc_free_context(m_context, 1);
		DebugMsg("DebugCommand","Samba!");
	}	

	static void xb_smbc_log(const char* msg)
	{
	}

	static void xb_smbc_auth(const char *srv, const char *shr, char *wg, int wglen, char *un, int unlen, char *pw, int pwlen)
	{
	  return;
	}
};