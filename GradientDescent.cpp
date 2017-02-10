#include "stdafx.h"
#include "GradientDescent.h"
#include "LevenbergMarquardt.h"
//#include "GaussJordan.h"
#include <cassert>
#include <fstream>
using namespace std;
#include "Utility.h"

bool
GradientDescent(const vector<real>& vvalues, //given values
				vector<real>& vEstimates, //parameters to be derived
				real (*func)(const vector<real>&, int n,  const void*), //function for evaluation
				surreal (*funcD)(const vector<surreal>&, int n, const void*), //function for derivatives
				int maxIter,
				real weight,
				const void* pParam, //general purpose parameters for modeling
				const real* plBounds, //lower bounds of each estimate
				const real* puBounds,  //upper bounds of each estimate
				const real* pWeights,	//weights for each square term
				ostream* out
				)
{
	int n = vvalues.size();
	int np = vEstimates.size();
	vector<real> verror(n);
	int i, j, k;
	//ofstream out("LMlog.txt", ios::app);

	//first find out the amount of discrepancy
	real error = 0;
	for(j=0; j<n; ++j)
	{
		real val = func(vEstimates, j, pParam);
		verror[j] = vvalues[j] - val;
		if(pWeights)
		{
			verror[j] *= pWeights[j];
		}
		error += verror[j]*verror[j];
	}

	for(i=0; i<maxIter; ++i)
	{
		//compute the gradient and Hessian
		vector<real> vGrad(np, 0);
		vector<surreal> vderive(np);
		vector<surreal> vdtmp(np);
		for(j=0; j<n; ++j)
		{
			int m, l;
			for(m=0; m<np; ++m)
			{
				for(l=0; l<np; ++l)
				{
					vdtmp[l].setReal(vEstimates[l]);
					if(l==m)
					{
						vdtmp[l].setImag(1.0);
					}
					else
					{
						vdtmp[l].setImag(0.0);
					}
				}
				vderive[m] = funcD(vdtmp, j, pParam);
			}
			for(m = 0; m < np; ++m)
			{
				vGrad[m] -= verror[j] * Imag(vderive[m]);
			}
		}
		if(out && i==0)
		{
			PrintGradient(*out, vGrad, np);
		}

		//update the parameter temporarily
		vector<real> vDelta = vGrad;

		//make sure that the resulting estimates will be within given bounds
		real rateDescent = 1.0;
		if(puBounds != NULL)
		{
			for(j=0; j<np; ++j)
			{
				real slack;
				real a = 1.0;
				if(vDelta[j] < 0) //note that vDelta will be subtracted
				{
					slack = puBounds[j] - vEstimates[j];
					if(slack < -vDelta[j])
					{
						a = slack / (-vDelta[j]);
						rateDescent = Min(rateDescent,a);
					}
				}
			}
		}
		if(plBounds != NULL)
		{
			for(j=0; j<np; ++j)
			{
				real slack;
				real a = 1.0;
				if(vDelta[j] > 0) //note that vDelta will be subtracted
				{
					slack = vEstimates[j] - plBounds[j];
					if(slack < vDelta[j])
					{
						a = slack / vDelta[j];
						rateDescent = Min(rateDescent,a);
					}
				}
			}
		}
		if(rateDescent < 0)
		{
			//cerr << "LevenbergMarquardt: negative descent rate of " << rateDescent << " found.\n";
			break;
		}

		real alpha = rateDescent;
		for(k=0; k<10; k++)
		{
			alpha *= 0.5; 
			vector<real> vEstimates2 = vEstimates;
			for(j=0; j<np; ++j)
			{
				vEstimates2[j] -= alpha * vDelta[j];
			}

			//check if the update improve the model
			vector<real> verror2(n);
			real error2 = 0;
			for(j=0; j<n; ++j)
			{
				real val = func(vEstimates2, j, pParam);
				verror2[j] = vvalues[j] - val;
				if(pWeights)
				{
					verror2[j] *= pWeights[j];
				}
				error2 += verror2[j]*verror2[j];
			}

			cout << alpha << ": " << error << " -> " << error2;
			cout << " " << vEstimates[4] << " vs " << vEstimates2[4] << endl;
			//take an appropriate action 
			if(error2 < error)
			{
				error = error2;
				verror = verror2;
				vEstimates = vEstimates2;
				//break;
			}
		}
	}

	//out.close();
	return true;
}
