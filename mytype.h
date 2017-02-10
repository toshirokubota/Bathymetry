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

typedef double real;
typedef unsigned char uchar;

typedef vector<real> vReal;
typedef vector<int> vInt;

const real fNaN =-9.9999999e37;
const double NaN = -9.99999999999e307;

const string strInvalid = "Invalid";

#endif /* mytype_h */
