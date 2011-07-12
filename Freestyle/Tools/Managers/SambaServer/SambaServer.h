#pragma once
#include "../../Threads/cthread.h"


using namespace std;

class SambaServer : public CThread
{
private:
	SambaServer();          // Private constructor
	~SambaServer() {}
	SambaServer(const SambaServer&);                 // Prevent copy-construction
	SambaServer& operator=(const SambaServer&);      // Prevent assignment
	void CreateConfigFile();

	class nmbdThread : public CThread
	{
	public:
		nmbdThread();
		unsigned long Process(void* parameter);
	};

	
	nmbdThread nmbdthread;
public:
	static SambaServer& getInstance()
	{
		static SambaServer singleton;
		return singleton;
	}
	static void setHostname(const string& hostname);
	unsigned long Process(void* parameter);
};
