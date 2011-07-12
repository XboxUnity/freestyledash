#include "stdafx.h"
#include "StatusMonitor.h"
#include "../../HTTP/HTTPDownloader.h"
#include "../../FTP/FTPServer.h"
#include "../../Settings/Settings.h"
#include "../../ContentList/ContentManager.h"
#include "../../Managers/TitleUpdates/TitleUpdateManager.h"
#include "../../XlinkKai/KaiClient.h"

// Add access to the smb lib
#include "../../../../Libs/libsmbd/xbox/xbLibSmb.h"

void StatusMonitor::tick()
{
	UpdateStatus();
	UpdateTemps();
}
void StatusMonitor::InitData()
{
	m_IP = "N/A";
	m_Status="";
	m_CPUTemp = 0;
	m_GPUTemp = 0;
	m_CPUTPercent = 0;
	m_GPUTPercent = 0;
	m_MEMTemp = 0;
	m_CASETemp = 0;
	m_CASETPercent = 0;
	m_MEMTPercent = 0;
	m_Date ="N/A";
	m_Time ="N/A";
	UpdateIP();
}
float StatusMonitor::GetCPUTemp()
{
	return m_CPUTemp;
}
float StatusMonitor::GetGPUTemp()
{
	return m_GPUTemp;
}
float StatusMonitor::GetMEMTemp()
{
	return m_MEMTemp;
}
float StatusMonitor::GetCASETemp()
{
	return m_CASETemp;
}
int StatusMonitor::GetCPUTPercent()
{
	return m_CPUTPercent;
}
int StatusMonitor::GetGPUTPercent()
{
	return m_MEMTPercent;
}

int StatusMonitor::GetMEMTPercent()
{
	return m_MEMTPercent;
}
int StatusMonitor::GetCASETPercent()
{
	return m_CASETPercent;
}
void StatusMonitor::UpdateTemps()
{
	bool useCelsius;
	float tmp;
	
	if(SETTINGS::getInstance().getCels() == 1)
		useCelsius = true;
	else
		useCelsius = false;

	m_CPUTemp = m_SMC.GetTemperature(CPU, useCelsius);
	tmp = m_CPUTemp;
	if(!useCelsius) 
		tmp = (m_CPUTemp - 32.0f) * 5.0f/9.0f;
	m_CPUTPercent = (int) (tmp * 10.0f);

	m_GPUTemp = m_SMC.GetTemperature(GPU, useCelsius);
	tmp = m_GPUTemp;
	if(!useCelsius) 
		tmp = (m_GPUTemp - 32.0f) * 5.0f/9.0f;
	m_GPUTPercent = (int) (tmp * 10.0f);

	m_MEMTemp = m_SMC.GetTemperature(MEM, useCelsius);
	tmp = m_MEMTemp;
	if(!useCelsius) 
		tmp = (m_MEMTemp - 32.0f) * 5.0f/9.0f;
	m_MEMTPercent = (int) (tmp * 10.0f);
		
	m_CASETemp = m_SMC.GetTemperature(BRD, useCelsius);
	tmp = m_CASETemp;
	if(!useCelsius) 
		tmp = (m_CASETemp - 32.0f) * 5.0f/9.0f;
	m_CASETPercent = (int) (tmp * 10.0f);

}
string StatusMonitor::GetStatus()
{
	return m_Status;
}
string StatusMonitor::GetIP()
{
	return m_IP;
}
void StatusMonitor::UpdateIP()
{
	m_IP = "N/A";
	XNADDR xnaddr;
	IN_ADDR ipaddr, lipaddr;
	XNetGetTitleXnAddr( &xnaddr );
	ipaddr=xnaddr.ina;
	char szip[16];
	if (memcmp(&ipaddr, &lipaddr, sizeof(ipaddr))!=0)
	{
		XNetInAddrToString( xnaddr.ina, szip, 16);
		m_IP = sprintfaA("%s", szip);
	}
	//return ipa;
}
void StatusMonitor::UpdateStatus()
{
	m_Status = "";
	string httpStatus = HTTPDownloader::getInstance().getStatus();
	if(HTTPDownloader::getInstance().getStatus() == "")
	{
	}
	else

	{
		m_Status ="HTTP : " +HTTPDownloader::getInstance().getStatus();
	}
		
	//string ftpStatus = "";
	if(CFTPServer::getInstance().HasActiveConnection())
	{
	
		if(m_Status == "")
		{
			m_Status = "FTP : "+CFTPServer::getInstance().getStatus();
		}
		else
		{
			m_Status = "FTP : "+CFTPServer::getInstance().getStatus()+" | " + m_Status;
		}
	}
	if(strcmp(ContentManager::getInstance().GetCurrentStatus().c_str(), "") != 0)
	{
		if(m_Status == "")
		{
			m_Status = "Scan : "+ ContentManager::getInstance().GetCurrentStatus();
		}
		else
		{
			m_Status = "Scan : "+ ContentManager::getInstance().GetCurrentStatus() + " | " + m_Status;
		}
	}
	if(strcmp(TitleUpdateManager::getInstance().getStatus().c_str(), "") != 0)
	{
		if(m_Status == "")
		{
			m_Status = "TU : "+ TitleUpdateManager::getInstance().getStatus();
		}
		else
		{
			m_Status = "TU : "+ TitleUpdateManager::getInstance().getStatus() + " | " + m_Status;
		}
	}
	if(getSambaClientCount() > 0 ) // there is status to display
	{
		if(m_Status == "")
		{
			m_Status = sprintfaA("smb : clients connected: %d", getSambaClientCount());
		}
		else
		{
			m_Status = sprintfaA("smb : clients connected: %d", getSambaClientCount()) + " | " + m_Status;
		}
	}
	if(CKaiClient::getInstance().IsEngineConnected() == true )  // there is status to diplay
	{
		if(m_Status == "")
		{
			m_Status = "Xlink Kai : Connected";
		}
		else
		{
			m_Status = (string)"Xlink Kai : Connected" + " | " + m_Status;
		}
	}
}