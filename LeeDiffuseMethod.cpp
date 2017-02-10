#include "LeeDiffuseMethod.h"
#include "LevenbergMarquardt.h"
#include "GradientDescent.h"
#include "Utility.h"
#include <cstdlib>

CLeeDiffuseMethod::CLeeDiffuseMethod(const char* paramFilename, real alpha, int numiter): CLeeMethod(paramFilename)
{
	m_Alpha = alpha;
	m_NumIter = numiter;
	/*for(int i=0; i<5; ++i)
	{
		LeeDiffusion::LeeDiffuseWeights[i] = 1.0; //fLeeScaleWidth[i];
	}*/
}

real
_computeRRS_LeeDiffuse(const vector<real>& vmodels, int n, const void* p)
{
	const CParamContainer* pParam = (CParamContainer *) p;
	int m = pParam->GetLength(pParam->GetName(0));
	if(n < m)
	{
		return _computeRRS_Lee(vmodels, n, p);
	}
	else
	{  //diffusion part. return the model parameter
		int k = (n - m) % vmodels.size();
		return vmodels[k];
	}
}

surreal
_computeRRSD_LeeDiffuse(const vector<surreal>& vmodels, int n, const void* p)
{
	const CParamContainer* pParam = (CParamContainer *) p;
	int m = pParam->GetLength(pParam->GetName(0));
	if(n < m)
	{
		return _computeRRSD_Lee(vmodels, n, p);
	}
	else
	{  //diffusion part. return the model parameter
		int k = (n - m) % vmodels.size();
		return vmodels[k];
	}
}

bool 
CLeeDiffuseMethod::_SetErrorWeights()
{
	int l = m_Estimate.NumParams();
	int m = m_Image.NumBands();
	int n = m + LeeDiffusion::NumNeighbors * l;
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
	for(int i=0; i<m; ++i)
	{
		if(_SupportedWavelength(m_Image.GetSpectra(i)))
		{
			m_pWeights[i] = 1.0;
		}
		else
		{
			m_pWeights[i] = 0.0;
		}
	}

	for(int i=m; i<n; ++i)
	{
		m_pWeights[i] = m_Alpha * LeeDiffusion::LeeDiffuseWeights[(i-m) % l];
	}

	return true;
}

bool
CLeeDiffuseMethod::Initialize()
{
	//set an initial value of each variable to the average of estimates obtained with Lee.
	bool bRet = CLeeMethod::Initialize();
	if(!bRet)
		return false;

	bRet = CLeeMethod::DoInversion();
	if(!bRet)
		return false;

	int i, j, k;
	for(k=0; k<m_Estimate.NumParams(); ++k) {
		real sum = 0;
		for(i=0; i<m_Image.NumRows(); ++i) {
			for(j=0; j<m_Image.NumCols(); ++j) {
				real val = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j);
				sum += val;
			}
		}
		real av = sum / (m_Image.NumRows() * m_Image.NumCols());
		for(i=0; i<m_Image.NumRows(); ++i) {
			for(j=0; j<m_Image.NumCols(); ++j) {
				m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, av);
			}
		}
	}
}

//#define LEE_DIFFUSE_JACOBI
#define LEE_DIFFUSE_SOLVER LevenbergMarquardt //GradientDescent //

bool
CLeeDiffuseMethod::DoInversion()
{
	bool bSuccess =	true; //CLeeMethod::DoInversion();
	if(!bSuccess)
		return false;
	//ofstream out("logLeeD.txt", ios::out);

	int i, j, k;
	int n = m_Image.NumBands();
	int l = m_Estimate.NumParams();
	int nn = LeeDiffusion::NumNeighbors;
	vector<real> vRRS(n + LeeDiffusion::NumNeighbors * l);
	vector<real> vEstimates(l);
	for(int niter=0; niter<m_NumIter && bSuccess; ++niter)
	{
#ifdef LEE_DIFFUSE_JACOBI
		CParamContainer tmpEstimate = m_Estimate; ////!!!
#endif
		real errSum = 0;
		for(i=0; i<m_Image.NumRows(); ++i)
		{
			for(j=0; j<m_Image.NumCols(); ++j)
			{
				//out << i << ", " << j << endl;

				real rrs_sum = 0;
				for(k=0; k<n; ++k)
				{
					vRRS[k] = m_Image.GetPixel(k,i,j);
					rrs_sum += vRRS[k] * vRRS[k];
				}
				for(k=0; k<LeeDiffusion::NumNeighbors; ++k)
				{
					int xn = j + LeeDiffusion::xoff[k];
					int yn = i + LeeDiffusion::yoff[k];
					int offset = n+k*l;
					if(xn >= 0 && xn < m_Image.NumCols() && yn >= 0 && yn < m_Image.NumRows())
					{
						for(int k2=0; k2<l; ++k2)
						{
							vRRS[offset + k2] = m_Estimate.GetParameter(k2, yn*m_Image.NumCols()+xn);
							//cout << k2 << ": " << m_Estimate.GetParameter(k2, i*m_Image.NumCols()+j);
							//cout << " vs. " << vRRS[offset + k2] << endl;
						}
					}
					else
					{
						for(int k2=0; k2<l; ++k2)
						{
							vRRS[offset + k2] = m_Estimate.GetParameter(k2, i*m_Image.NumCols()+j);
						}
					}
				}

				for(k=0; k<l; ++k)
				{
					vEstimates[k] = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j);
				}

				real rate = 0.001;
				int maxIter = 10;
				bool bRet = LEE_DIFFUSE_SOLVER(vRRS, vEstimates,
					_computeRRS_LeeDiffuse, _computeRRSD_LeeDiffuse, 
					maxIter, rate,
					(void *) &m_InterpParam, 
					m_pLowerBounds, m_pUpperBounds,
					m_pWeights,
					0);
				if(!bRet)
				{
					//cerr << "LevenbergMarquardt exception. at (" << i << ", " << j << ")\n";
					bSuccess = false;
				}
				for(k=0; k<l; ++k)
				{
					real df = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j) - vEstimates[k];
					errSum += df * df;
#ifdef LEE_DIFFUSE_JACOBI
					tmpEstimate.SetParameter(k, i*m_Image.NumCols()+j, vEstimates[k]); ////!!!
#else
					m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, vEstimates[k]);
#endif
				}
			}
		}
		//cout << niter << "\t" << errSum << "\n";
#ifdef LEE_DIFFUSE_JACOBI
		m_Estimate = tmpEstimate; ////!!!
#endif
	}
	//out.close();
	if(!bSuccess)
	{
		printf("LevenbergMarquardt exception. at (%d, %d)\n", i, j);
	}
	return bSuccess;
}
