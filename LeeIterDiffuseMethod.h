#ifndef ___LEE_ITER_DIFFUSE_METHOD_H___
#define ___LEE_ITER_DIFFUSE_METHOD_H___

#include "Stdafx.h"
#include "LeeMethod.h"

class CLeeIterDiffuseMethod: public CLeeMethod
{
public:
	CLeeIterDiffuseMethod(const char* paramFilename = 0, real speed=0.5, int iter = 5);

	//virtual bool Initialize();
	virtual bool DoInversion();
protected:
	real m_Speed;
	int m_NumIter;
};

#endif /* ___LEE_ITER_DIFFUSE_METHOD_H___ */