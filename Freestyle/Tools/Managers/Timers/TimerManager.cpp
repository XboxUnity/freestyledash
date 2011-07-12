#include "stdafx.h"

#include "TimerManager.h"
#include "../../Generic/xboxtools.h"
#include "../../Debug/Debug.h"
using namespace std;
#define TIMER_PRECISION 100
unsigned long TimerManager::Process(void* parameter)
{
	SetThreadName("TimerManager");
	bool doneLooping = false;
	while(!doneLooping)
	{
		Sleep(TIMER_PRECISION);
		DWORD currentTick = GetTickCount();
		int x =0;
		for(TimerItem::const_iterator it = _observers.begin(); it != _observers.end(); ++it)
		{
			
			DWORD lastTick = _observersLastRun.find(it->first)->second;
			if(lastTick + it->second <=currentTick)
			{
				//DebugMsg("TimerManager","Notify %d",x);
				_observersLastRun[it->first] =GetTickCount();
			
				it->first->tick();
				//DebugMsg("TimerManager","Done Notify %d",x);
			}
			x ++;		
		}
		
	}
	return 0;
}
TimerManager::TimerManager() 
{	DebugMsg("TimerManager","Init Timer Manager");
	this->CreateThread(CPU1_THREAD_1);
}         
void TimerManager::add(iTimerObserver& ref, int IntervalInMS)
{

	_observers.insert(TimerItem::value_type(&ref,IntervalInMS));
	

	_observersLastRun.insert(TimerLastRun::value_type(&ref,GetTickCount()));
}
void TimerManager::remove(iTimerObserver& ref)
{
	_observers.erase(&ref);
	_observersLastRun.erase(&ref);

}
