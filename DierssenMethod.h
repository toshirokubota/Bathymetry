#ifndef ___DIERSSEN_METHOD_H___
#define ___DIERSSEN_METHOD_H___

#include "Stdafx.h"
#include <fstream>
#include <vector>
using namespace std;

#include "AbstractMethod.h"

class CDierssenMethod: public CAbstractMethod
{
public:
	CDierssenMethod(const char* paramFilename = 0);

	virtual bool Initialize();
	virtual bool DoInversion();
	virtual bool PostProcess();

protected:
};

real
_computeDierssenDepth(real rs555, real rs670, real c1, real c2, real c3, real m, real b);

#endif /* ___DIERSSEN_METHOD_H___ */
