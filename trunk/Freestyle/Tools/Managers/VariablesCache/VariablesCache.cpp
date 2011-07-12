#include "stdafx.h"

#include "../../Generic/Tools.h"
#include "../../Generic/XboxTools.h"
#include "../../Debug/Debug.h"

#include "VariablesCache.h"

bool VariablesCache::variableExist(string name)
{
	std::map<string,string>::iterator itr = m_Variables.find(name);
	if(itr ==m_Variables.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}


void VariablesCache::DumpVariablesToDebug()
{
	DebugMsg("VariablesCache","Current VariablesCache :");
	std::map<string,string>::iterator itr = m_Variables.begin();
	for(;itr != m_Variables.end();itr++)
	{
		DebugMsg("VariablesCache","%s - %s",itr->first.c_str(),itr->second.c_str());
	}

}

string VariablesCache::getVariable(string name,string defaultValue)
{
	if(variableExist(name))
	{
		return m_Variables[name];
	}
	else
	{
		return defaultValue;
	}
}
void VariablesCache::setVariable(string name,string value)
{
	m_Variables[name] = value;
}
