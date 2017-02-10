#include "ParamInterface.h"
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;
#include "Utility.h"

const vector<real> g_vEmpty(0);

CParamContainer::CParamContainer(int n, int m)
{
	if(n)
	{
		m_vNames.resize(n, "");
		if(m)
		{
			vector<real> vnans(m, fNaN);
			m_vParameters.resize(n, vnans);
		}
		else
		{
			m_vParameters.resize(n);
		}
	}
}

CParamContainer::CParamContainer(const CParamContainer& param)
{
	int n = param.NumParams();
	m_vNames.clear();
	m_vParameters.clear();
	for(int i=0; i<n; ++i)
	{
		m_vNames.push_back(param.GetName(i));
		m_vParameters.push_back(param.GetParameter(param.GetName(i)));
	}
}

const CParamContainer&
CParamContainer:: operator = (const CParamContainer& param)
{
	int n = param.NumParams();
	m_vNames.clear();
	m_vParameters.clear();
	for(int i=0; i<n; ++i)
	{
		m_vNames.push_back(param.GetName(i));
		m_vParameters.push_back(param.GetParameter(param.GetName(i)));
	}

	return *this;
}

int 
CParamContainer::GetLength(const string& name) const
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return -1;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		return m_vParameters[k].size();
	}
}

bool 
CParamContainer::Resize(const string& name, int size)
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return false;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		m_vParameters[k].resize(size, fNaN);
		return true;
	}
}

const vReal& 
CParamContainer::GetParameter(const string& name) const
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return g_vEmpty;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		return m_vParameters[k];
	}
}

real 
CParamContainer::GetParameter(int k, int n) const
{
	assert(k <= m_vParameters.size());
	if(n<m_vParameters[k].size() && n>=0)
	{
		return m_vParameters[k][n];
	}
	else
	{
		return fNaN;
	}
}

real 
CParamContainer::GetParameter(const string& name, int n) const
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return fNaN;
	}
	else
	{
		int k = p - m_vNames.begin();
		return GetParameter(k, n);
	}
}

real 
CParamContainer::GetParameterInterpolate(const string& target, const string& reference, real refVal) const
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), reference);
	if(p==m_vNames.end())
	{
		return fNaN;
	}
	vector<string>::const_iterator q = find(m_vNames.begin(), m_vNames.end(), target);
	if(q==m_vNames.end())
	{
		return fNaN;
	}
	
	int i;
	int k = p - m_vNames.begin();
	for(i=0; i<m_vParameters[k].size(); ++i)
	{
		if(m_vParameters[k][i] > refVal)
		{
			break;
		}
	}
	int m = q - m_vNames.begin();
	if(i == 0)
	{
		return m_vParameters[m][0];
	}
	else if(i>=m_vParameters[k].size())
	{
		return m_vParameters[m][i-1];
	}
	else
	{
		int i2 = Max(0, i-1);
		real v1 = m_vParameters[m][i2];
		real v2 = m_vParameters[m][i];
		real r = m_vParameters[k][i] - m_vParameters[k][i2];
		assert(r>0);
		real t = (refVal - m_vParameters[k][i2]) / r;
		return v1 * (1.0-t) + v2 * t;
	}
}


bool 
CParamContainer::SetParameter(const string& name, const vReal& data)
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return false;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		m_vParameters[k] = data;
		return true;
	}
}

bool 
CParamContainer::SetParameter(int k, int n, real data)
{
	assert(k <= m_vParameters.size());
	if(n<m_vParameters[k].size() && n>=0)
	{
		m_vParameters[k][n] = data;
		return true;
	}
	else
	{
		return false;
	}
}

bool 
CParamContainer::SetParameter(const string& name, int n, real data)
{
	vector<string>::const_iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return false;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		return SetParameter(k, n, data);
	}
}

bool 
CParamContainer::AddValue(const string& name, real data)
{
	vector<string>::iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return false;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		m_vParameters[k].push_back(data);
		return true;
	}
}

bool 
CParamContainer::Clear(const string& name)
{
	vector<string>::iterator p = find(m_vNames.begin(), m_vNames.end(), name);
	if(p==m_vNames.end())
	{
		return false;
	}
	else
	{
		int k = p - m_vNames.begin();
		assert(k <= m_vParameters.size());
		m_vNames.erase(p);
		m_vParameters.erase(m_vParameters.begin() + k);
		return true;
	}
}


CParamInterface::CParamInterface()
{
}

bool 
CParamInterface::OpenParameter(const char* paramfile)
{
	ifstream in(paramfile);
	if(in.fail())
	{
		cerr << "OpenParameter:: Failed to open " << paramfile << " for read." << endl;
		return false;
	}

	int numdata;
	in >> numdata;
	int nump;
	in >> nump;

	m_Container = CParamContainer(nump, numdata);

	vector<string> names(nump);
	int i, j;
	for(i=0; i<nump; ++i)
	{
		in >> names[i];
		m_Container.SetName(i, names[i]);
		if(in.bad())
		{
			return false;
		}
	}
	for(i=0; i<numdata; ++i)
	{
		for(j=0; j<nump; ++j)
		{
			real val;
			in >> val;
			m_Container.SetParameter(names[j], i, val);
			if(in.bad())
			{
				return false;
			}
		}
	}

	return true;
}

