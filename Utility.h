#ifndef ___UTILITY_H___
#define ___UTILITY_H___
#include "StdAfx.h"
#include "mytype.h"
#include "image.h"
#include <math.h>

#define Max(a,b) (((a)>(b))? (a): (b))
#define Min(a,b) (((a)<(b))? (a): (b))

template<class Item>
inline Item Abs(Item a) {return ((a>0) ? a: -a);}
template<class Item>
inline Item Sign(Item a) {return ((a>0) ? 1: ((a==0)? 0: -1));}

inline int Round(real a) {return (int) ((a) > 0 ? ((a) + 0.5):((a) - 0.5));}

#undef PI
const real PI=atan2(0,-1.0);
const real TwoPi=2.0*PI;
const real HalfPi=.5*PI;
const real QtrPi=.25*PI;
const real Qtr3Pi=.75*PI;

bool _SupportedWavelength(real r);

RealImage
GaussianFilter1D(real gauss_sgm, real epsilon);

void
DiffusionFit(RealImage& f, const RealImage& g, real rate = 0.5, real difspeed = 1.0);

#endif /* ___UTILITY_H___ */
