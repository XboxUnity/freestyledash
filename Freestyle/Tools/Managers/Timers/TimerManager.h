#pragma once
#include "../../Threads/cthread.h"
class iTimerObserver{
public :
	virtual void tick() =0;
};


class TimerManager:CThread
{
public:
  static TimerManager& getInstance()
  {
    static TimerManager singleton;
    return singleton;
  }

	
	void add(iTimerObserver& ref, int IntervalInMS);
	
	void remove(iTimerObserver& ref);

 
// Other non-static member functions
private:

   

	std::map<iTimerObserver* const,DWORD> _observersLastRun;
	typedef std::map<iTimerObserver* const, DWORD > TimerLastRun;
	std::map<iTimerObserver* const,int const> _observers;
	typedef std::map<iTimerObserver* const, int const> TimerItem;
	

  TimerManager();          // Private constructor
  ~TimerManager() {}
  TimerManager(const TimerManager&);                 // Prevent copy-construction
  TimerManager& operator=(const TimerManager&);      // Prevent assignment
  unsigned long TimerManager::Process(void* parameter);
};
