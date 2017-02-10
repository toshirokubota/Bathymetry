#include "LeeIterDiffuseMethod.h"
#include "LevenbergMarquardt.h"
#include "GradientDescent.h"
#include "Utility.h"
#include <cstdlib>

CLeeIterDiffuseMethod::CLeeIterDiffuseMethod(const char* paramFilename, real speed, int numiter): CLeeMethod(paramFilename)
{
	m_Speed = speed;
	m_NumIter = numiter;
}

void
_DiffuseParams(CParamContainer& params, int rows, int cols, real alpha)
{
	int i, j, k;
	for(k=0; k<params.NumParams(); ++k) {
		for(i=1; i<rows-1; ++i) {
			for(j=1; j<cols-1; ++j) {
				real here = params.GetParameter(k, i*cols+j);
				real north = params.GetParameter(k, (i-1)*cols+j);
				real south = params.GetParameter(k, (i+1)*cols+j);
				real east = params.GetParameter(k, i*cols+(j+1));
				real west = params.GetParameter(k, i*cols+(j-1));

				real val=here+ alpha *((north-here)+(south-here)+\
					(east-here)+(west-here));
				params.SetParameter(k, i*cols+j, val);
			}
		}
	}
}

//#define LEE_DIFFUSE_JACOBI
#define LEE_DIFFUSE_SOLVER LevenbergMarquardt //GradientDescent //

bool
CLeeIterDiffuseMethod::DoInversion()
{
	bool bSuccess;
	for(int niter=0; niter<m_NumIter; ++niter)
	{
		bSuccess =	CLeeMethod::DoInversion();
		if(!bSuccess)
		{
			return false;
		}
		_DiffuseParams(m_Estimate, m_Image.NumRows(), m_Image.NumCols(), m_Speed);
	}

	return true;
}
