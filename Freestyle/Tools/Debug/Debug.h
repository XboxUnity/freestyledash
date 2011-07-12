#pragma once

#define LOG_PATH "Game:\\Data\\Logs\\"
#define LOG_FILE "debug.log"

//using namespace std;
#ifndef CSQLITE
bool StartDebugger(int PortSend,int PortRecieve);
bool IsDebugMode();
void EnableDebugMode();
void EndDebugger();
void SetDebugDisplay(bool toggle);
void DebugMsg(const WCHAR* filter, const WCHAR* message, ...);
void ForcedDebugMsg(const char* filter, const char* message, ...);
#endif

#ifdef __cplusplus
    extern "C" {
#endif
void DebugMsg(const char* filter, const char* message, ...);
void DebugMsgRaw(const char* filter, const char* message);
#ifdef __cplusplus
	}
#endif


