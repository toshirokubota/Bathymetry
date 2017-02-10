#ifndef ___LEVENBERG_MARQUARDT_H___
#define ___LEVENBERG_MARQUARDT_H___

#include "StdAfx.h"
#include "mytype.h"
#include <vector>
using namespace std;

#include "derivify.h"

void
PrintGradient(ostream& out, const vector<real>& v, int n);

void
PrintHessian(ostream& out, const vector<real>& v, int n);

/*bool
LevenbergMarquardt(const vector<real>& vvalues, //given values
				   const vector<real>& vParams, //static parameters
				   vector<real>& vEstimates, //parameters to be derived
				   int stride, //# of static parameters to for each function evaluation
				   real (*func)(vector<real>::const_iterator pParam, const vector<real>& vEstimates), //function for evaluation
				   surreal (*funcD)(vector<real>::const_iterator pParam, const vector<surreal>& vEstimates), //function for derivatives
				   int maxIter,
				   real weight);*/

/*bool
LevenbergMarquardtOld(const vector<real>& vvalues, //given values
				   const vector<real>& vStatParams, //static parameters
				   vector<real>& vEstimates, //parameters to be derived
				   int stride, //# of static parameters to for each function evaluation
				   real (*func)(vector<real>::const_iterator, const vector<real>&), //function for evaluation
				   surreal (*funcD)(vector<real>::const_iterator, const vector<surreal>&), //function for derivatives
				   int maxIter,
				   real weight,
				   const real* lBounds = 0, //lower bounds of each estimate
				   const real* uBounds = 0,  //upper bounds of each estimate
				   const real* pSigma = 0	//weights for each square term
				   );*/

/* 
New interface - makes more sense.
*/
bool
LevenbergMarquardt(const vector<real>& vvalues, //given values
				   vector<real>& vEstimates, //parameters to be derived
				   real (*func)(const vector<real>&, int n,  const void*), //function for evaluation
				   surreal (*funcD)(const vector<surreal>&, int n, const void*), //function for derivatives
				   int maxIter,
				   real weight,
				   const void* pParam = 0, //general purpose parameters for modeling
				   const real* lBounds = 0, //lower bounds of each estimate
				   const real* uBounds = 0,  //upper bounds of each estimate
				   const real* pSigma = 0,	//weights for each square term
				   ostream* out = 0
				   );

#endif /* ___LEVENBERG_MARQUARDT_H___ */