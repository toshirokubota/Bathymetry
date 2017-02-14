#include "DierssenDiffuseMethod.h"

#include "DierssenMethod.h"
#include "Utility.h"
#include "derivify.h"
#include "LevenbergMarquardt.h"

CDierssenDiffuseMethod::CDierssenDiffuseMethod(const char* paramFilename, real alpha, int numiter): CDierssenMethod(paramFilename)
{
	m_Alpha = alpha;
	m_NumIter = numiter;
}

bool
CDierssenDiffuseMethod::DoInversion()
{
	int i, j, k;
	int l = m_Estimate.NumParams();
	RealImage f(l, m_Image.NumRows(), m_Image.NumCols());
	for(k=0; k<l; ++k)
	{
		for(i=0; i<m_Image.NumRows(); ++i)
		{
			for(j=0; j<m_Image.NumCols(); ++j)
			{
				real v = m_Estimate.GetParameter(k, i*m_Image.NumCols() + j);
				f.SetPixel(k,i,j, v);
			}
		}
	}
	RealImage g = f;
	real rate = 1.0;
	for(k=0; k<m_NumIter; ++k)
	{
		DiffusionFit(f, g, m_Alpha);
	}
	for(k=0; k<l; ++k)
	{
		for(i=0; i<m_Image.NumRows(); ++i)
		{
			for(j=0; j<m_Image.NumCols(); ++j)
			{
				m_Estimate.SetParameter(k, i*m_Image.NumCols() + j, f.GetPixel(k, i, j));
			}
		}
	}

	bool bSuccess =	CDierssenMethod::DoInversion();
	return bSuccess;
}
