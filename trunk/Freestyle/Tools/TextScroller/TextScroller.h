#pragma once

#include "../Managers/Timers/TimerManager.h"
struct StatusInfo
{
	string feedTitle;
	string itemTitle;
	string itemDescription;
	string itemPubDate;
};
class iStatusInformer{
public :
	virtual StatusInfo getStatus() =0;
};
class iStatusDisplayer{
public :
	virtual void setStatus(StatusInfo inf) =0;
};
class TextScroller:public iTimerObserver
{
private :
	
  TextScroller();          // Private constructor
  ~TextScroller() {}
  TextScroller(const TextScroller&);                 // Prevent copy-construction
  TextScroller& operator=(const TextScroller&);      // Prevent assignment
  vector<iStatusInformer*> m_Informers;
  vector<iStatusDisplayer*> m_Displayers;
  int m_CurrentInformerIndex;
public :
	static TextScroller& getInstance()
	{
		static TextScroller singleton;
		return singleton;
	}
	void RegisterInformer(iStatusInformer* newItem)
	{
		m_Informers.push_back(newItem);
	}
	void RegisterDisplayer(iStatusDisplayer* newItem)
	{
		m_Displayers.push_back(newItem);
	}
	void tick();
	


};