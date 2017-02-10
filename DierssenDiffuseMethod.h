#ifndef ___DIERSSEN_DIFFUSE_METHOD_H___
#define ___DIERSSEN_DIFFUSE_METHOD_H___

#include "Stdafx.h"
#include <fstream>
#include <vector>
using namespace std;

#include "DierssenMethod.h"
	
class CDierssenDiffuseMethod: public CDierssenMethod
{
public:
	CDierssenDiffuseMethod(const char* paramFilename = 0, real alpha=0.5, int iter = 5);

	//virtual bool Initialize();
	virtual bool DoInversion();

protected:
	real m_Alpha;
	int m_NumIter;
};

#endif /* ___DIERSSEN_DIFFUSE_METHOD_H___ */
