#ifndef ___ABSTRACT_METHOD_H___
#define ___ABSTRACT_METHOD_H___
#include "Stdafx.h"
#include <fstream>
#include <vector>
using namespace std;

#include "HyperspectralImage.h"
#include "DataInterface.h"
#include <Windows.h>

class CAbstractMethod
{
public:
	CAbstractMethod()
	{
		m_Tick.QuadPart = 0;
		//LARGE_INTEGER freq;
		//if(QueryPerformanceCounter(&freq))
		//	cout << "CAbstractMethod:: " << freq.HighPart << " " << freq.LowPart << endl;
		//else
		//	cout << "CAbstractMethod:: no high performance frequency counter." << endl;
	}

	bool SetData(const CHyperspectralImage& data)
	{
		m_Image = data;
		return true;
	}
	const CHyperspectralImage& GetData() const
	{
		return m_Image;
	}

	int NumParameters() const {return m_Params.NumParams();}
	int LengthParameter(const string& name) const
	{
		return m_Params.GetLength(name);
	}
	const string& GetParameterName(int i)
	{
		if(i>=0 && i<m_Params.NumParams())
		{
			return m_Params.GetName(i);
		}
		else
		{
			return strInvalid;
		}
	}

	bool SetParameter(const CParamContainer& param)
	{
		m_Params = param;
		return true;
	}
	vector<real> GetParameter(const string& name) const
	{
		return m_Params.GetParameter(name);
	}
	bool SetParameter(const string& name, const vector<real>& vdata)
	{
		return m_Params.SetParameter(name, vdata);
	}
	real GetParameter(const string& name, int n) const
	{
		return m_Params.GetParameter(name, n);
	}
	bool SetParameter(const string& name, int n, real val)
	{
		return m_Params.SetParameter(name, n, val);
	}

	int NumEstimates() const {return m_Estimate.NumParams();}
	const string& GetEstimateName(int i)
	{
		if(i>=0 && i<m_Estimate.NumParams())
		{
			return m_Estimate.GetName(i);
		}
		else
		{
			return strInvalid;
		}
	}	
	
	vector<real> GetEstimate(const string& name) const
	{
		return m_Estimate.GetParameter(name);
	}
	real GetEstimate(const string& name, int n) const
	{
		return m_Estimate.GetParameter(name, n);
	}

	virtual bool Initialize() = 0;
	virtual bool DoInversion() = 0;
	virtual bool PostProcess() { return true;};

	//timer function
	bool SetTimer()
	{
		if(QueryPerformanceCounter(&m_Tick))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	double ReadTimer() const //read the elapsed time in msec
	{
		LARGE_INTEGER tick, freq;
		if(QueryPerformanceCounter(&tick) && QueryPerformanceFrequency(&freq))
		{
			double scale = pow(2.0, 32.0);
			double fFreq = freq.HighPart * scale + freq.LowPart;
			double t1 = m_Tick.HighPart * scale + m_Tick.LowPart;
			double t2 = tick.HighPart * scale + tick.LowPart;
			double mult = 1000.0;
			double res = mult * (t2 - t1) / fFreq;
			return res;
		}
		else
		{
			return -1;
		}
	}

protected:
	CParamContainer		m_Estimate;	//inversion estimates
	CParamContainer		m_Params;	//parameters
	CHyperspectralImage	m_Image;	//image data
	LARGE_INTEGER m_Tick;
};

#endif /* ___ABSTRACT_METHOD_H___ */