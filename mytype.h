#ifndef _mytype_h_
#define _mytype_h_

//#include <iostream>
//#include <fstream>
#include <vector>
#include <string>
//#include <algorithm>
using namespace std;

#define _BOOL
#ifndef _BOOL
enum bool { false, true};
#define _BOOL
#endif

#define _REAL_IS_DOUBLE
#ifdef _REAL_IS_DOUBLE
typedef double real;
#else
typedef float real;
#endif

typedef unsigned char uchar;

typedef vector<real> vReal;
typedef vector<int> vInt;

const real fNaN =std::numeric_limits<float>::quiet_NaN();
const double dNaN = std::numeric_limits<double>::quiet_NaN();
#ifdef _REAL_IS_DOUBLE
const real NaN = dNaN;
#else
const real NaN = fNaN;
#endif
const string strInvalid = "Invalid";

#endif /* mytype_h */
