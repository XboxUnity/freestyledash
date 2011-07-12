#pragma once

class VariablesCache
{
protected :
	std::map<string, string> m_Variables;
	VariablesCache() {}  // Private constructor
	~VariablesCache() {}
	VariablesCache(const VariablesCache&);                 // Prevent copy-construction
	VariablesCache& operator=(const VariablesCache&);      // Prevent assignment

	bool variableExist(string name);

public :
	void DumpVariablesToDebug();
	string getVariable(string name,string defaultValue);
	void setVariable(string name,string value);

	static VariablesCache& getInstance()
	{
		static VariablesCache singleton;
		return singleton;
	}

};