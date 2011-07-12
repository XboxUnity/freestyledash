#include "stdafx.h"
#include "smc.h"


#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
void smc::PrepareBuffers()
{
 	//Zero our mem
	ZeroMemory( m_SMCMessage, sizeof(m_SMCMessage) );
	ZeroMemory( m_SMCReturn, sizeof(m_SMCReturn) );
}


//Usage: command is one of the POWER_LED constants from smc_constant.h
//         animate is true for ring LED startup light sequence
void smc::SetPowerLED(unsigned char command, bool animate)
{
    PrepareBuffers();
	
    m_SMCMessage[0] = 0x8c;
    m_SMCMessage[1] = command;
    m_SMCMessage[2] = (animate ? 0x01 : 0x00);
	HalSendSMCMessage(m_SMCMessage, NULL);
}


//Usage: color is one of LED constants from smc_constant.h
void smc::SetLEDS(LEDState sTopLeft, LEDState sTopRight, LEDState sBottomLeft, LEDState sBottomRight)
{
	PrepareBuffers();
    m_SMCMessage[0] = 0x99;
    m_SMCMessage[1] = 0x01;

	m_SMCMessage[2] = ((unsigned char)(sTopLeft>>3) | (unsigned char)(sTopRight>>2) | (unsigned char)(sBottomLeft>>1) | (unsigned char)(sBottomRight));

	HalSendSMCMessage(m_SMCMessage, NULL);
}

float smc::GetTemperature(TEMP_INDEX sTempIndex, bool Celsius)
{
	PrepareBuffers();
    m_SMCMessage[0] = REQUEST_TEMP;
    HalSendSMCMessage(m_SMCMessage, m_SMCReturn);
	
	float temp = (float)((m_SMCReturn[sTempIndex * 2 + 1] | (m_SMCReturn[sTempIndex * 2 +2] <<8)) / 256.0);

    if(!Celsius) temp = (9.0f/5.0f) * temp + 32.0f;
	
	return temp;
}

char* smc::GetSMCVersion()
{
	PrepareBuffers();
    m_SMCMessage[0] = REQUEST_SMC_VERSION;
    HalSendSMCMessage(m_SMCMessage, m_SMCReturn);
	
	static char version[5] = "";
	sprintf_s(version, sizeof(version), "%d.%d", m_SMCReturn[2], m_SMCReturn[3]);

    return version;
}

TILT_STATE smc::GetTiltState()
{
	PrepareBuffers();
    m_SMCMessage[0] = REQUEST_TILT;
    HalSendSMCMessage(m_SMCMessage, m_SMCReturn);
	
	TILT_STATE sTiltState = ((m_SMCReturn[1] & 1) ? VERTICAL : HORIZONTAL);
	return sTiltState;
}

unsigned char smc::GetAVPack()
{
	//MATTIE: names for the types of av packs
	PrepareBuffers();
    m_SMCMessage[0] = REQUEST_AV_PACK;
	HalSendSMCMessage(m_SMCMessage, m_SMCReturn);
	
	return m_SMCReturn[1];
}

TRAY_STATE smc::GetTrayState()
{
	PrepareBuffers();
    m_SMCMessage[0] = REQUEST_TRAY_STATE;
	HalSendSMCMessage(m_SMCMessage, m_SMCReturn);
	
	return (TRAY_STATE)((m_SMCReturn[1] & 0xF) % 5);
}
void smc::OpenTray()
{
	PrepareBuffers();
    m_SMCMessage[0] = 0x8b;
	m_SMCMessage[1] = 0x60;
	HalSendSMCMessage(m_SMCMessage, NULL);
	DebugMsg("SMC","Open tray");
	
}
void smc::CloseTray()
{
	PrepareBuffers();
    m_SMCMessage[0] = 0x8b;
	m_SMCMessage[1] = 0x62;
	HalSendSMCMessage(m_SMCMessage, NULL);
	DebugMsg("SMC","Close tray");
}
void smc::SetFanSpeed(int fan, int speed)
{
	PrepareBuffers();
	m_SMCMessage[0] = fan ? 0x94 : 0x89;

	// Fail safe to keep a speed set above 100 in the xml from
	//  stopping the fans.
	if (speed > 100)
		speed = 100;

	if (speed < 45)
	{
		m_SMCMessage[1] = 0x7F;
	}
	else
	{
		m_SMCMessage[1] = (unsigned char)speed | 0x80;
	}


	HalSendSMCMessage(m_SMCMessage, NULL);
}

// Thanks goto Aaron for this
// Setting Names to AV Packs
const char* smc::GetAVPackName()
{
	switch (GetAVPack()) {
		case AV_HDMI: return "HDMI";
		case AV_COMPONENT: return "Component";
		case AV_VGA: return "VGA";
		case AV_COMPOSITETV: return "Composite TV";
		case AV_COMPOSITEHD: return "Composite HDTV";
		case AV_HDMIAUDIO: return "HDMIw/Opt";
		default: return NULL;
	}
}
