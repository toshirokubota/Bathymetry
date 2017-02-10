#ifndef ___LEE_POSTPROCESS_METHOD_H___
#define ___LEE_POSTPROCESS_METHOD_H___

#include "Stdafx.h"
#include "LeeMethod.h"

class CLeePostprocessMethod: public CLeeMethod
{
public:
	CLeePostprocessMethod(const char* paramFilename = 0, real sigma=1.0);

	//virtual bool Initialize();
	virtual bool DoInversion();
protected:
	real m_Sigma;
};

#endif /* ___LEE_POSTPROCESS_METHOD_H___ */