#ifndef ___LEE_DIFFUSE_METHOD_VER2_H___
#define ___LEE_DIFFUSE_METHOD_VER2_H___

#include "Stdafx.h"
#include "LeeDiffuseMethod.h"

class CLeeDiffuseMethodVer2: public CLeeDiffuseMethod
{
public:
	CLeeDiffuseMethodVer2(const char* paramFilename = 0, real alpha=0.00005, int iter = 5);

	//virtual bool Initialize();
	virtual bool DoInversion();
protected:
	virtual bool _SetErrorWeights();
	virtual bool _SetBounds();
};


#endif /* ___LEE_DIFFUSE_METHOD_VER2_H___ */