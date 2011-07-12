#include "stdafx.h"
#include "SambaServer.h"
#include "../../Generic/xboxtools.h"
#include "../../Settings/Settings.h"
#include "../../Managers/Drives/DrivesManager.h"
#include "../../Managers/Drives/Drive.h"
#include "../../../../Libs/libsmbd/xbox/xbLibSmb.h"

extern void xb_smbc_log(const char* msg);

SambaServer::nmbdThread::nmbdThread() {
	set_log_callback(xb_smbc_log);
	setHostname(SETTINGS::getInstance().getHostname().c_str());
	init_samba_server(SAMBA_ERROR_LEVEL);
	CreateThread(CPU2_THREAD_1);
}

unsigned long SambaServer::nmbdThread::Process(void *parameter) 
{ 
	xb_setSambaWorkgroup(SETTINGS::getInstance().getSambaClientWorkgroup().c_str());
	
	start_nmbd_loop();		// start a nmbd on a seperate thread...this goes on forever, and shouldn't ever end
	return 1;
}

SambaServer::SambaServer(void)
{
	CreateConfigFile();
	mkdir("game:\\tmp");
	CreateThread(CPU3_THREAD_1);
}

void SambaServer::CreateConfigFile()
{
	FILE* cf = fopen("game:\\smb.conf", "wb");
	fputs("[global]", cf);				fputc(0x0A, cf);
	fputs("security = share", cf);		fputc(0x0A, cf);
	fputs("share modes = no", cf);		fputc(0x0A, cf);
	fputs("", cf);						fputc(0x0A, cf);

	std::vector<Drive* const> mountedDrives;

	DrivesManager::getInstance().getMountedDrives(&mountedDrives);
	for(unsigned int x=0;x<mountedDrives.size();x++)
	{
		string d = "[" + mountedDrives[x]->GetCleanDriveName() + "]";
		fputs(d.c_str(), cf);				fputc(0x0A, cf);
		d = "path = " + mountedDrives[x]->GetCleanDriveName() + ":";
		fputs(d.c_str(), cf);			fputc(0x0A, cf);

		if (mountedDrives[x]->GetCleanDriveName().compare("Flash") != 0) {// make Flash read only
			fputs("readonly = no", cf);			fputc(0x0A, cf);
		}
		fputs("guest ok = yes", cf);		fputc(0x0A, cf);
		fputs("", cf);						fputc(0x0A, cf);
	}

	fputs("[FSD]", cf);					fputc(0x0A, cf);
	fputs("path = GAME:", cf);			fputc(0x0A, cf);
	fputs("readonly = no", cf);			fputc(0x0A, cf);
	fputs("guest ok = yes", cf);		fputc(0x0A, cf);
	fputs("", cf);						fputc(0x0A, cf);

/*	fputs("[Content]", cf);					fputc(0x0A, cf);
	fputs("path = Hdd1:\\Content", cf);			fputc(0x0A, cf);
	fputs("readonly = no", cf);			fputc(0x0A, cf);
	fputs("guest ok = yes", cf);		fputc(0x0A, cf);
	fputs("", cf);						fputc(0x0A, cf);
*/

	fclose(cf);	
}


void SambaServer::setHostname(const string& hostname) {
	xb_setSambaHostname(hostname.c_str());
}

unsigned long SambaServer::Process(void *parameter) 
{
	while (true) {
		externmain();
	}
	return 1;
}
