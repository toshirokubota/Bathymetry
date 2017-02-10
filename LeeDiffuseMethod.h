#ifndef ___LEE_DIFFUSE_METHOD_H___
#define ___LEE_DIFFUSE_METHOD_H___

#include "Stdafx.h"
#include "LeeMethod.h"

class CLeeDiffuseMethod: public CLeeMethod
{
public:
	CLeeDiffuseMethod(const char* paramFilename = 0, real alpha=0.00005, int iter = 5);

	virtual bool Initialize();
	virtual bool DoInversion();
protected:
	virtual bool _SetErrorWeights();
	//real _estimateDiffusionWeight();
	real m_Alpha;
	int m_NumIter;
};


namespace LeeDiffusion
{
	const int NumNeighbors = 4;
	const int xoff[NumNeighbors] = {0, -1, 1, 0};
	const int yoff[NumNeighbors] = {-1, 0, 0, 1};
	const real LeeDiffuseWeights[5] = {1.0, 1.0, 1.0, 1.0, 1.0};
	const int xoff0[NumNeighbors+1] = {0, 0, -1, 1, 0};
	const int yoff0[NumNeighbors+1] = {0, -1, 0, 0, 1};
};

#endif /* ___LEE_DIFFUSE_METHOD_H___ */