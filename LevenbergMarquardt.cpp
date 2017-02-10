#include "stdafx.h"
#include "LevenbergMarquardt.h"
#include "GaussJordan.h"
#include <cassert>
#include <fstream>
using namespace std;
#include "Utility.h"

void
PrintGradient(ostream& out, const vector<real>& v, int n)
{
	int i;
	out << "Gradient: " << endl;
	for(i=0; i<n; ++i)
	{
		out << v[i] << " ";
	}
	out << endl;
}

void
PrintHessian(ostream& out, const vector<real>& v, int n)
{
	int i, j;
	out << "Hessian: " << endl;
	//out.precision(3);
	for(i=0; i<n; ++i)
	{
		for(j=0; j<n; ++j)
		{
			out << v[i*n+j] << "\t";
		}
		out << endl;
	}
}

bool
LevenbergMarquardt(const vector<real>& vvalues, //given values
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

	//first find out the amount of discrepancy
	real error = 0;
	//cout << "LevenbergMarquardt (Entry):" << endl;
	for(j=0; j<n; ++j)
	{
		real val = func(vEstimates, j, pParam);
		verror[j] = vvalues[j] - val;
		if(pWeights)
		{
			verror[j] *= pWeights[j];
		}
		error += verror[j]*verror[j];
		//cout << j << ": " << verror[j] << endl;
	}

	for(i=0; i<maxIter; ++i)
	{
		//compute the gradient and Hessian
		vector<real> vHessian(np*np, 0);
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
				for(l = 0; l < np; ++l)
				{
					real hs;
					if(l==m)
					{
						hs = (1+weight) * Imag(vderive[m]) * Imag(vderive[l]);
					}
					else
					{
						hs = Imag(vderive[m]) * Imag(vderive[l]);
					}
					if(pWeights)
					{
						vHessian[m*np+l] += hs * pWeights[j] * pWeights[j];
					}
					else
					{
						vHessian[m*np+l] += hs;
					}
				}
			}
		}
		if(i==0 && out)
		{
			*out << n << ", " << np << endl;
			PrintGradient(*out, vGrad, np);
			PrintHessian(*out, vHessian, np);
		}
		//update the parameter temporarily
		vector<real> vDelta = vGrad;
		if(!GaussJordan(vHessian, vDelta, np))
		{
			return false;
		}

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
		if(rateDescent <= 0)
		{
			//cerr << "LevenbergMarquardt: negative descent rate of " << rateDescent << " found.\n";
		}

		vector<real> vEstimates2 = vEstimates;
		real val;
		for(j=0; j<np; ++j)
		{
			vEstimates2[j] -= rateDescent * vDelta[j];
			val = vEstimates2[j];
		}

		//check if the update improve the model
		vector<real> verror2(n);
		real error2 = 0;
		//cout << "LevenbergMarquardt (" << i << "):" << endl;
		for(j=0; j<n; ++j)
		{
			real val = func(vEstimates2, j, pParam);
			verror2[j] = vvalues[j] - val;
			if(pWeights)
			{
				verror2[j] *= pWeights[j];
			}
			error2 += verror2[j]*verror2[j];
			//cout << j << ": " << verror2[j] << endl;
		}
		if(out)
		{
			*out << i << ": rate = " << rateDescent << " weight = " << weight;
			*out << " error = " << error << ", error2 = " << error2;
			*out << (error > error2 ? " updated" : "") << endl;
			*out << "Estimate: [";
				for(int i2=0; i2<vEstimates2.size(); ++i2)
				{
					*out << vEstimates2[i2] << " ";
				}
				*out << "]" << endl;
		}

		//take an appropriate action 
		if(error2 < error)
		{
			vEstimates = vEstimates2;
			error = error2;
			verror = verror2;
			weight *= 0.1;
		}
		else
		{
			weight *= 10.0;
		}
	}

	if(out)
	{
		*out << "Results: [";
		for(i=0; i<vEstimates.size(); ++i)
		{
			*out << vEstimates[i] << " ";
		}
		*out << "]" << endl;
	}
	return true;
}

