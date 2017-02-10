#include "LeeDiffuseMethod.h"
#include "LevenbergMarquardt.h"
#include "GradientDescent.h"
#include "Utility.h"
#include <cstdlib>

CLeeDiffuseODEMethod::CLeeDiffuseODEMethod(const char* paramFilename, real alpha, int numiter): CLeeDiffuseMethod(paramFilename, alpha, numiter)
{
}

bool
CLeeDiffuseMethod::DoInversion()
{
	bool bSuccess =	true; //CLeeMethod::DoInversion();
	//ofstream out("logLeeD.txt", ios::out);

	int i, j, k;
	int n = m_Image.NumBands();
	int l = m_Estimate.NumParams();
	int nn = LeeDiffusion::NumNeighbors;
	vector<real> vRRS(n);
	vector<real> vEstimates(l);
	vector<real> vParamsNN(l * nn);
	for(int niter=0; niter<m_NumIter; ++niter)
	{
		real errSum = 0;
		for(i=0; i<m_Image.NumRows(); ++i)
		{
			for(j=0; j<m_Image.NumCols(); ++j)
			{
				for(k=0; k<n; ++k)
				{
					vRRS[k] = m_Image.GetPixel(k,i,j);
				}
				for(k=0; k<LeeDiffusion::NumNeighbors; ++k)
				{
					int xn = j + LeeDiffusion::xoff[k];
					int yn = i + LeeDiffusion::yoff[k];
					if(!(xn >= 0 && xn < m_Image.NumCols() && yn >= 0 && yn < m_Image.NumRows()))
					{
						xn = j;
						yn = i;
					}
					for(int k2=0; k2<l; ++k2)
					{
						vParamsNN[k * l + k2] = m_Estimate.GetParameter(k2, yn*m_Image.NumCols()+xn);
					}
				}
				for(k=0; k<l; ++k)
				{
					vEstimates[k] = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j);
				}

				real rate = 0.001;
				int maxIter = 10;
				bool bRet = ReactionDiffusion(vRRS, vEstimates, vParamsNN,
					_computeRRS_LeeDiffuse, _computeRRSD_LeeDiffuse, 
					maxIter, rate,
					(void *) &m_InterpParam, 
					m_pLowerBounds, m_pUpperBounds,
					m_pWeights,
					0);
				if(!bRet)
				{
					cerr << "LevenbergMarquardt exception. at (" << i << ", " << j << ")\n";
					bSuccess = false;
				}
				for(k=0; k<l; ++k)
				{
					real df = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j) - vEstimates[k];
					errSum += df * df;
					m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, vEstimates[k]);
				}
			}
		}
		//cout << niter << "\t" << errSum << "\n";
	}
	//out.close();

	return bSuccess;
}
