// Bathymetry.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <vector>
using namespace std;

#include "mytype.h"
#include "LevenbergMarquardt.h"
#include "derivify.h"
#include "rndm.h"
#include "LeeMethod.h"

const real TrueValues[] = {0.02, 2.0, 0.12, 5.0, 0.04};
//const int NumStatParameters = 6;
const int NumEstParameters = 5;


real
_compute(vector<real>::const_iterator p, const vector<real>& vparam)
{
	real lamb = *p;
	real a0 = *(p+1);
	real a1 = *(p+2);
	real aw = *(p+3);
	real alb = *(p+4);
	real bbw = *(p+5);
	real P = vparam[0];
	real G = vparam[1];
	real BP = vparam[2];
	real B = vparam[4];
	real H = vparam[3];

	real at=aw+P*(a0+ a1 * log(P)) + G * exp(-0.015 * (lamb - 440));
	real bb =  BP * pow((double)(400 / lamb),0.018932636) + bbw; //BP * sqrt(400 / lamb); //+ bbw;
	real u = bb / (at + bb); 
	real karpa = at + bb;

	real Duc = 1.03 * sqrt(1 + 2.4 * u);
	real Dub = 1.05 * sqrt(1 + 5.5 * u);

	real rrs_c = (0.084 + 0.17 * u) * u * (1 - exp(-(1.2 + Duc) * karpa * H));
	real rrs_b = 0.3183 * pow(B, 0.5) * alb * exp(-(1.2 + Dub) * karpa * H);

	real term1 = rrs_c + rrs_b;
	real term2 = 0.52 * term1 / (1 - 1.6 * term1);

	/*cout << "u = " << u << "\t";
	cout << "rrs_b = " << rrs_b << "\t";
	cout << "rrs_c = " << rrs_c << "\t";
	cout << "term2 = " << term2 << "\t";
	cout << endl;*/
	//term2=aw*P*P+a0*G*G+a1*BP*BP+alb*B*B+bbw*H*H;
	return term2;
}

surreal
_computeD(vector<real>::const_iterator p, const vector<surreal>& vparam)
{
	real lamb = *p;
	real a0 = *(p+1);
	real a1 = *(p+2);
	real aw = *(p+3);
	real alb = *(p+4);
	real bbw = *(p+5);
	surreal P = vparam[0];
	surreal G = vparam[1];
	surreal BP = vparam[2];
	surreal B = vparam[4];
	surreal H = vparam[3];

	surreal at=aw+P*(a0+ a1 * log(P)) + G * exp(-0.015 * (lamb - 440));
	surreal bb =  BP * pow((double)(400 / lamb),0.018932636) + bbw; //BP * sqrt(400 / lamb); //+ bbw;
	surreal u = bb / (at + bb); 
	surreal karpa = at + bb;

	surreal Duc = 1.03 * sqrt(1 + 2.4 * u);
	surreal Dub = 1.05 * sqrt(1 + 5.5 * u);

	surreal rrs_c = (0.084 + 0.17 * u) * u * (1 - exp(-(1.2 + Duc) * karpa * H));
	surreal rrs_b = 0.3183 * pow(B, 0.5) * alb * exp(-(1.2 + Dub) * karpa * H);

	surreal term1 = rrs_c + rrs_b;
	surreal term2 = 0.52 * term1 / (1 - 1.6 * term1);

	//term2=aw*P*P+a0*G*G+a1*BP*BP+alb*B*B+bbw*H*H;
	return term2;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i, j, k;
	int numT = NumEstParameters;
	vector<real> vTrue(numT);
	for(i=0; i<numT; ++i)
	{
		vTrue[i] = TrueValues[i];
	}
	vector<real> vEstimates(numT);
	for(i=0; i<numT; ++i)
	{
		real er = 0.2 * (rndm(0) - 0.5);
		vEstimates[i] = TrueValues[i] * (1 + er);
	}
	cout << "Initial: " << endl;
	for(i=0; i<numT; ++i)
	{
		cout << TrueValues[i] << " vs " << vEstimates[i] << endl;
	}

	CLeeMethod method("C:\\Kubota\\Research\\HyperSpectralImages\\truth\\Lee_model_parameters_0306_2007.txt");
	int numData = method.LengthParameter(method.GetParameterName(0))-10;
	int numParams = method.NumParameters();
	vector<real> vParams(numParams * numData);
	for(i=0, k=0; i<numData; ++i)
	{
		for(j=0; j<numParams; ++j, ++k)
		{
			vParams[k] = method.GetParameter(method.GetParameterName(j), i);
		}
	}

	vector<real> vData(numData);
	for(i=0; i<numData; ++i)
	{
		real er = 0.1 * (rndm(0) - 0.5);
		vData[i] = _compute(vParams.begin() + i*numParams, vTrue) * (1 + er);
	}

	real weight = 0.001;
	int maxIter = 50;
	bool bRet = LevenbergMarquardt(vData, vParams, vEstimates, numParams,
		_compute, _computeD, maxIter, weight);


	cout << "Results: " << endl;
	for(i=0; i<numT; ++i)
	{
		cout << TrueValues[i] << " vs " << vEstimates[i] << endl;
	}

	return 0;
}

