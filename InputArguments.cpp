#include "InputArguments.h"
#include <cstdlib>
using namespace std;

CInputArguments::CInputArguments(int argc, TCHAR* argv[])
{
	vParam.clear();
	Read(argc, argv);
}

bool
CInputArguments::Read(int argc, TCHAR* argv[])
{
	int i;
	for(i=0; i<argc; ++i)
	{
		vParam.push_back(argv[i]);
	}
	return true;
}

int
CInputArguments::Get(const char* prestr, int defval)
{
	int i, j;
	for(i=0, j=0; i<vParam.size(); ++i)
	{
		if(vParam[i] == prestr)
		{
			if(i<vParam.size()-1)
			{
				return atoi(vParam[i+1].c_str());
			}
		}
	}

	return defval;
}

real
CInputArguments::Get(const char* prestr, real defval)
{
	int i, j;
	for(i=0, j=0; i<vParam.size(); ++i)
	{
		if(vParam[i] == prestr)
		{
			if(i<vParam.size()-1)
			{
				return (real) strtod(vParam[i+1].c_str(), NULL);
			}
		}
	}

	return defval;
}

const char* 
CInputArguments::Get(const char* prestr, const char* defval)
{
	int i, j;
	for(i=0, j=0; i<vParam.size(); ++i)
	{
		if(vParam[i] == prestr)
		{
			if(i<vParam.size()-1)
			{
				return vParam[i+1].c_str();
			}
		}
	}

	return defval;
}

bool 
CInputArguments::Get(const char* prestr)
{
	int i, j;
	for(i=0, j=0; i<vParam.size(); ++i)
	{
		if(vParam[i] == prestr)
		{
			return true;
		}
	}

	return false;
}

