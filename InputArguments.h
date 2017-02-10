#ifndef ___INPUT_ARGUMENTS_H___
#define ___INPUT_ARGUMENTS_H___
#include "StdAfx.h"
#include "mytype.h"
#include <vector>
#include <string>
#include <fstream>
using namespace std;

enum param_type {PARAM_INT, PARAM_REAL, PARAM_STRING};

class CInputArguments
{
public:
	CInputArguments(int argc=0, TCHAR* argv[] = NULL);

	int Get(const char* prestr, int defval);
	real Get(const char* prestr, real defval);
	const char* Get(const char* prestr, const char* defval);
	bool Get(const char* prestr);
	bool Read(int argc, TCHAR* argv[]);
	//Usage(ostream& out);
	//Printf(ostream& out);
protected:
	vector<string> vParam;
};

#endif /* ___INPUT_ARGUMENTS_H___ */
