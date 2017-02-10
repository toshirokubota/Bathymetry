#ifndef ___PARAM_INTERFACE_H___
#define ___PARAM_INTERFACE_H___

#include "stdafx.h"
#include <vector>
#include <string>
#include <cassert>
#include "mytype.h"

using namespace std;

class CParamContainer
{
public:
	CParamContainer(int n = 0, int m = 0);
	CParamContainer(const CParamContainer& param);
	const CParamContainer& operator = (const CParamContainer& param);

	int NumParams() const {return m_vNames.size();}
	const string& GetName(int n) const
	{
		assert(n>=0 && n<NumParams());
		return m_vNames[n];
	}
	bool SetName(int n, const string& name)
	{
		if(n>=0 && n<NumParams())
		{
			m_vNames[n] = name;
			return true;
		}
		else
		{
			return false;
		}
	}
	bool AddName(const string& name)
	{
		m_vNames.push_back(name);
		vector<real> v;
		m_vParameters.push_back(v);
		return true;
	}

	int GetLength(const string& name) const;

	bool Resize(const string& name, int size);
	const vReal& GetParameter(const string& name) const;
	real GetParameter(int k, int n) const;
	real GetParameter(const string& name, int n) const;
	real GetParameterInterpolate(const string& target, const string& reference, real refVal) const;
	bool SetParameter(const string& name, const vReal& data);
	bool SetParameter(int k, int n, real data);
	bool SetParameter(const string& name, int n, real data);
	bool AddValue(const string& name, real data);
	bool Clear(const string& name);
	void Clear()
	{
		m_vNames.clear();
		m_vParameters.clear();
	}

protected:
	vector<string>		m_vNames;	
	vector<vReal>		m_vParameters;	
};

class CParamInterface
{
public:
	CParamInterface();
	bool OpenParameter(const char* paramfile);
	const CParamContainer& GetContainer() const {return m_Container;}

protected:
	CParamContainer m_Container;
};

#endif /* ___PARAM_INTERFACE_H___ */