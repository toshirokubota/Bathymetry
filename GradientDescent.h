#ifndef ___GRADIENT_DESCENT_H___
#define ___GRADIENT_DESCENT_H___

#include "StdAfx.h"
#include "mytype.h"
#include <vector>
using namespace std;

#include "derivify.h"

bool
GradientDescent(const vector<real>& vvalues, //given values
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

#endif /* ___GRADIENT_DESCENT_H___ */