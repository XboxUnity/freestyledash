#pragma once
#include "stdafx.h"
#include "../../Managers/Timers/TimerManager.h"
#include "../../SMC/Smc.h"
class StatusMonitor:public iTimerObserver
{
public :
	static StatusMonitor& getInstance()
	{
		static StatusMonitor singleton;
		return singleton;
	}
	void tick();
	string GetStatus();
	string GetIP();
	float GetCPUTemp();
	float GetGPUTemp();
	float GetMEMTemp();
	float GetCASETemp();
	int GetCPUTPercent();
	int GetGPUTPercent();
	int GetMEMTPercent();
	int GetCASETPercent();
private :
	smc m_SMC;
	string m_IP;
	string m_Status;
	float m_CPUTemp;
	float m_GPUTemp;
	float m_MEMTemp;
	float m_CASETemp;

	int m_CASETPercent;
	int m_MEMTPercent;
	int m_CPUTPercent;
	int m_GPUTPercent;
	
	string m_Date;
	string m_Time;
	void InitData();
	void UpdateStatus();
	void UpdateIP();
	void UpdateTemps();
	StatusMonitor() {

		InitData();
		TimerManager::getInstance().add(*this,100);
	}
	~StatusMonitor() {
		TimerManager::getInstance().remove(*this);
	}
	StatusMonitor(const StatusMonitor&);                 // Prevent copy-construction
	StatusMonitor& operator=(const StatusMonitor&);      // Prevent assignment
};