#ifndef ___LEE_DIFFUSE_ODE_METHOD_H___
#define ___LEE_DIFFUSE_ODE_METHOD_H___

#include "Stdafx.h"
#include "LeeDiffuseMethod.h"

class CLeeDiffuseODEMethod: public CLeeDiffuseMethod
{
public:
	CLeeDiffuseODEMethod(const char* paramFilename = 0, real alpha=0.00005, int iter = 5);

	//virtual bool Initialize();
	virtual bool DoInversion();
protected:
	real m_Alpha;
	int m_NumIter;
};

#endif /* ___LEE_DIFFUSE_ODE_METHOD_H___ */