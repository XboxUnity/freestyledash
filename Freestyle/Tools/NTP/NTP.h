#pragma once

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "../Threads/Cthread.h"

class NTP : public CThread
{

public:
	static NTP& getInstance()
	{
		static NTP singleton;
		return singleton;
	}
	void SyncTime();
protected:
private:

	unsigned long Process( void * parameter );
	void DoTimeSync();

	NTP() {};
	~NTP() {}
	NTP(const NTP&);                 // Prevent copy-construction
	NTP& operator=(const NTP&);      // Prevent assignment
};
