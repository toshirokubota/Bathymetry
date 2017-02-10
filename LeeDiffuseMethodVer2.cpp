#include "LeeDiffuseMethodVer2.h"
#include "LevenbergMarquardt.h"
#include "Utility.h"

CLeeDiffuseMethodVer2::CLeeDiffuseMethodVer2(const char* paramFilename, real alpha, int numiter): CLeeDiffuseMethod(paramFilename, alpha, numiter)
{

}

real
_computeRRS_LeeDiffuseVer2(const vector<real>& vmodels, int n, const void* p)
{
	const CParamContainer* pParam = (CParamContainer *) p;
	int m = pParam->GetLength(pParam->GetName(0));
	int nn = LeeDiffusion::NumNeighbors;
	int l = sizeof(LeeDiffusion::LeeDiffuseWeights) / sizeof(LeeDiffusion::LeeDiffuseWeights[0]);
	if(n < m * (nn + 1))
	{
		int n2 = n % m;
		int k = n / m;
		vector<real> vmodels2;
		vmodels2.insert(vmodels2.begin(), vmodels.begin()+k*l, vmodels.begin()+(k+1)*l);
		return _computeRRS_Lee(vmodels2, n2, p);
	}
	else
	{  //diffusion part.
		int n2 = n - m * (nn + 1);
		int l2 = n2 % l; //parameter id
		int m = n2 / l + 1; //neighbor id
		int k = (n - m) % vmodels.size();
		return vmodels[l2] - vmodels[m * l + l2];
	}
}

surreal
_computeRRSD_LeeDiffuseVer2(const vector<surreal>& vmodels, int n, const void* p)
{
	const CParamContainer* pParam = (CParamContainer *) p;
	int m = pParam->GetLength(pParam->GetName(0));
	int nn = LeeDiffusion::NumNeighbors;
	int l = sizeof(LeeDiffusion::LeeDiffuseWeights) / sizeof(LeeDiffusion::LeeDiffuseWeights[0]);
	if(n < m * (nn + 1))
	{
		int n2 = n % m;
		int k = n / m;
		vector<surreal> vmodels2;
		vmodels2.insert(vmodels2.begin(), vmodels.begin()+k*l, vmodels.begin()+(k+1)*l);
		return _computeRRSD_Lee(vmodels2, n2, p);
	}
	else
	{  //diffusion part.
		int n2 = n - m * (nn + 1);
		int l2 = n2 % l; //parameter id
		int m = n2 / l + 1; //neighbor id
		int k = (n - m) % vmodels.size();
		return vmodels[l2] - vmodels[m * l + l2];
	}
}

void
_AccessEstimate(vector<real>& vestimate, 
			 CParamContainer& param, 
			 int x, int y,
			 int height, int width,
			 bool bSet)
{
	int l = param.NumParams();
	int nn = LeeDiffusion::NumNeighbors;
	int nm = nn + 1;
	if(vestimate.size() < l * (nn + 1))
	{
		cerr << "CLeeDiffuseMethodVer2::_AccessEstimate: not enough space in vestimate.\n";
		return;
	}
	int i, j, k;
	for(i=0; i<nm; ++i)
	{
		int xn = x + LeeDiffusion::xoff0[i];
		int yn = y + LeeDiffusion::yoff0[i];
		if(xn < 0 || xn >= width)
		{
			xn = x;
		}
		if(yn < 0 || yn >= height)
		{
			yn = y;
		}
		for(k=0; k<l; ++k)
		{
			if(bSet)
			{
				param.SetParameter(k, yn*width+xn, vestimate[i*l + k]);
			}
			else
			{
				vestimate[i*l + k] = param.GetParameter(k, yn*width+xn);
			}
		}
	}
}

void
_SetRRS(vector<real>& vrrs,
		const CHyperspectralImage& image,
		CParamContainer& param, 
		int x, int y)
{
	int n = image.NumBands();
	int l = param.NumParams();
	int nn = LeeDiffusion::NumNeighbors;
	int nm = LeeDiffusion::NumNeighbors + 1;
	if(vrrs.size() < n * nm + nn * l)
	{
		cerr << "CLeeDiffuseMethodVer2::_SetRRS: not enough space in vrrs.\n";
		return;
	}
	int j, k;
	for(k=0; k<nm; ++k)
	{
		int xn = x + LeeDiffusion::xoff0[k];
		int yn = y + LeeDiffusion::yoff0[k];
		if(xn < 0 || xn >= image.NumCols())
		{
			xn = x;
		}
		if(yn < 0 || yn >= image.NumRows())
		{
			yn = y;
		}
		for(j=0; j<n; ++j)
		{
			vrrs[k*n+j] = image.GetPixel(j, yn, xn);
		}
	}
	for(k=nm * n; k< nm * n + nn * l; ++k)
	{
		vrrs[k] = 0;
	}
}

bool 
CLeeDiffuseMethodVer2::_SetErrorWeights()
{
	int l = m_Estimate.NumParams();
	int m = m_Image.NumBands();
	int nn = LeeDiffusion::NumNeighbors;
	int nm = LeeDiffusion::NumNeighbors + 1;
	int n = nm * m + LeeDiffusion::NumNeighbors * l;
	if(m_pWeights)
	{
		delete [] m_pWeights;
		m_pWeights = 0;
	}
	m_pWeights = new real[n];
	if(m_pWeights == 0)
	{
		return false;
	}
	for(int i=0; i<m * nm; ++i)
	{
		if(_SupportedWavelength(m_Image.GetSpectra(i % m)))
		{
			m_pWeights[i] = 1.0;
		}
		else
		{
			m_pWeights[i] = 0.0;
		}
	}
	for(int i=nm * m; i<n; ++i)
	{
		m_pWeights[i] = m_Alpha * LeeDiffusion::LeeDiffuseWeights[(i-nm*m) % l];
	}

	return true;
}

bool 
CLeeDiffuseMethodVer2::_SetBounds()
{
	int i;
	int l = m_Estimate.NumParams();
	int nm = LeeDiffusion::NumNeighbors + 1;
	if(m_pLowerBounds)
	{
		delete [] m_pLowerBounds;
		m_pLowerBounds = 0;
	}
	if(m_pUpperBounds)
	{
		delete [] m_pUpperBounds;
		m_pUpperBounds = 0;
	}
	m_pLowerBounds = new real[l * nm];
	if(m_pLowerBounds == 0)
	{
		return false;
	}
	m_pUpperBounds = new real[l * nm];
	if(m_pUpperBounds == 0)
	{
		delete [] m_pLowerBounds;
		m_pLowerBounds = 0;
		return false;
	}

	for(i=0; i<l * nm; ++i)
	{
		m_pLowerBounds[i] = (Lee_lower_bound[i % l] - fLeeScaleOffset[i % l]) / fLeeScaleWidth[i % l];
		m_pUpperBounds[i] = (Lee_upper_bound[i % l] - fLeeScaleOffset[i % l]) / fLeeScaleWidth[i % l];
	}
	return true;
}

#define LEE_DIFFUSE_SOLVER LevenbergMarquardt

bool
CLeeDiffuseMethodVer2::DoInversion()
{
	bool bSuccess =	true; //CLeeMethod::DoInversion();
	if(!bSuccess)
		return false;
	ofstream out("LMlog.txt", ios::out);
	out.close();

	int i, j, k, k2;
	int n = m_Image.NumBands();
	int l = m_Estimate.NumParams();
	int nn = LeeDiffusion::NumNeighbors;
	int nm = LeeDiffusion::NumNeighbors + 1;
	vector<real> vRRS(nm * n + nn * l);
	vector<real> vEstimates(nm * l);
	for(int niter=0; niter<m_NumIter; ++niter)
	{
		for(i=1; i<m_Image.NumRows()-1; ++i)
		{
			for(j=1; j<m_Image.NumCols()-1; ++j)
			{
				ofstream out("LMlog.txt", ios::app);
				out << i << ", " << j << endl;
				out.close();

				_SetRRS(vRRS, m_Image, m_Estimate, i, j);

				_AccessEstimate(vEstimates, m_Estimate, i, j, m_Image.NumRows(), m_Image.NumCols(), false);

				real rate = 0.001;
				int maxIter = 10;
				bool bRet = LEE_DIFFUSE_SOLVER(vRRS, vEstimates,
					_computeRRS_LeeDiffuseVer2, _computeRRSD_LeeDiffuseVer2, 
					maxIter, rate,
					(void *) &m_InterpParam, 
					m_pLowerBounds, m_pUpperBounds,
					m_pWeights);
				if(!bRet)
				{
					cerr << "LevenbergMarquardt exception. at (" << i << ", " << j << ")\n";
					bSuccess = false;
				}

				_AccessEstimate(vEstimates, m_Estimate, i, j, m_Image.NumRows(), m_Image.NumCols(), true);
			}
		}
	}
	return bSuccess;
}
