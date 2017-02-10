#ifndef ___DIERSSEN_POSTPROCESS_METHOD_H___
#define ___DIERSSEN_POSTPROCESS_METHOD_H___

#include "Stdafx.h"
#include "DierssenMethod.h"

class CDierssenPostprocessMethod: public CDierssenMethod
{
public:
	CDierssenPostprocessMethod(const char* paramFilename = 0, real sigma=1.0, int niter=5);

	//virtual bool Initialize();
	virtual bool DoInversion();
protected:
	real m_Alpha;
	int m_NumIter;
};

#endif /* ___LEE_POSTPROCESS_METHOD_H___ */