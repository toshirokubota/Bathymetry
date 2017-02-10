#ifndef ___RNDM_H____
#define ___RNDM_H____
#include "stdafx.h"

double rndm(int jd); //uniformly distributed in [0, 1]

double ran1(long *idum); //numerical recipe

double gasdev(long jd); //gaussian distributed

#endif /* ___RNDM_H____ */