#include "LeePostprocessMethod.h"
#include "Utility.h"
#include "Convolve.h"

CLeePostprocessMethod::CLeePostprocessMethod(const char* paramFilename, real sigma): CLeeMethod(paramFilename)
{
	m_Sigma = sigma;
}

bool
CLeePostprocessMethod::DoInversion()
{
	bool bSuccess = CLeeMethod::DoInversion();
	if(!bSuccess)
		return bSuccess;

	int i, j, k;
	int l = m_Estimate.NumParams();
	RealImage paramImage(l, m_Image.NumRows(), m_Image.NumCols());
	for(k=0; k<l; ++k)
	{
		for(i=0; i<m_Image.NumRows(); ++i)
		{
			for(j=0; j<m_Image.NumCols(); ++j)
			{
				real val = m_Estimate.GetParameter(k, i*m_Image.NumCols() + j);
				paramImage.SetPixel(k, i, j, val);
			}
		}
	}

	real epsilon = 0.01;
	RealImage gauss=GaussianFilter1D(m_Sigma,epsilon);
	paramImage=SeparableImageConvolution(paramImage,gauss,gauss);
	for(k=0; k<l; ++k)
	{
		for(i=0; i<m_Image.NumRows(); ++i)
		{
			for(j=0; j<m_Image.NumCols(); ++j)
			{
				m_Estimate.SetParameter(k, i*m_Image.NumCols() + j, paramImage.GetPixel(k, i, j));
			}
		}
	}
	
	return bSuccess;
}
