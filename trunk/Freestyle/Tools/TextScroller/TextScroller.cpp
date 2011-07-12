#include "stdafx.h"

#include "TextScroller.h"
#include "../Managers/Timers/TimerManager.h"

#define TEXTUPDATEINTERVAL 10.6

TextScroller::TextScroller() 
{	
	m_CurrentInformerIndex =0;
	TimerManager::getInstance().add(*this,(int)TEXTUPDATEINTERVAL*1000);
}     
void TextScroller::tick()
{
	int nSize = m_Informers.size();
	if(m_CurrentInformerIndex< nSize)
	{
		iStatusInformer* info = m_Informers.at(m_CurrentInformerIndex);
		StatusInfo newStatus = info->getStatus();
		nSize = m_Displayers.size();
		for(int x=0;x<nSize;x++)
		{
			iStatusDisplayer* displayer = m_Displayers.at(x);
			displayer->setStatus(newStatus);
		}
	}
	m_CurrentInformerIndex += 1;
	nSize = m_Informers.size();
	if(m_CurrentInformerIndex > nSize)
	{
		m_CurrentInformerIndex = 0;
	}
}