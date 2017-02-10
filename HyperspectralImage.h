#ifndef ___HYPERSPECTRAL_IMAGE_H___
#define ___HYPERSPECTRAL_IMAGE_H___
#include "StdAfx.h"
#include "Image.h"

#include <string>
using namespace std;

#include "ParamInterface.h"

class CHyperspectralImage: public Image<real>
{
public:
	CHyperspectralImage();
	CHyperspectralImage(const CHyperspectralImage& image);
	CHyperspectralImage& operator=(const CHyperspectralImage& image);
	CHyperspectralImage(const RealImage& image);
	CHyperspectralImage& operator=(const RealImage& image);

	//string GetFilename() const {return m_Filename;}
	//int GetTotalWidth() const {return m_Width;}
	//int GetTotalHeight() const {return m_Height;}
	int GetNumSpectra() const {return m_vLambda.size();}
	const CParamContainer& GetParameters() const {return m_Parameters;}

	const vector<real>& GetSpectra() const {return m_vLambda;}
	real GetSpectra(int n) const
	{
		assert(n>=0 && n<m_vLambda.size());
		return m_vLambda[n];
	}
	bool SetSpectra(const vector<real>& data) 
	{
		m_vLambda= data;
		return true;
	}
	bool SetSpectra(int n, real val)
	{
		if(n>=0 && n<m_vLambda.size())
		{
			m_vLambda[n] = val;
			return true;
		}
		else
		{
			return false;
		}
	}
	bool SetParameters(const CParamContainer& params) 
	{
		m_Parameters = params;
		return true;
	}
	vector<real> GetBands(int r, int c) const;
	bool SetBands(const vector<real>& vdata, int r, int c);

protected:
	vector<real>	m_vLambda;
	CParamContainer	m_Parameters;
};


#endif /* ___HYPERSPECTRAL_IMAGE_H___ */