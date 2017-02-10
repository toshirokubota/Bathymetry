#ifndef ___LEE_METHOD_H___
#define ___LEE_METHOD_H___

#include "Stdafx.h"
#include <fstream>
#include <vector>
using namespace std;

#include "AbstractMethod.h"

class CLeeMethod: public CAbstractMethod
{
public:
	CLeeMethod(const char* paramFilename = 0);

	virtual bool Initialize();
	virtual bool DoInversion();
	virtual bool PostProcess();
	bool LoadParameters(const char* filename);

protected:
	virtual bool _InitializeParameters();
	virtual bool _InitializeEstimates();
	virtual bool _InterpolateParameters();
	virtual bool _ScaleModels();
	virtual bool _ReScaleModels();
	virtual bool _SetBounds();
	virtual bool _SetErrorWeights();

	CParamContainer m_InterpParam;  //parameters interpolated at different wave bands.
	string m_ParameterFilename;
	int m_NumIter;

	real* m_pLowerBounds;
	real* m_pUpperBounds;
	real* m_pWeights;
};

#include "derivify.h"

real
_computeRRS_Lee(const vector<real>& vmodel, int n, const void* p);

surreal
_computeRRSD_Lee(const vector<surreal>& vmodel, int n, const void* p);

const real Lee_upper_bound[5]={0.5, 3.5, 0.5, 0.9, 10};
const real Lee_lower_bound[5]={0.006, 0.01, 0.0005, 0.001, 0.2};

//#define SCALE_MODEL
#ifdef SCALE_MODEL
const real fLeeScaleWidth[] = {0.5, 3.5, 0.5, 0.9, 10.0};
const real fLeeScaleOffset[] = {0, 0, 0, 0, 0};
#else
const real fLeeScaleWidth[] = {1.0, 1.0, 1.0, 1.0, 1.0};
const real fLeeScaleOffset[] = {0, 0, 0, 0, 0};
#endif

#endif /* ___LEE_METHOD_H___ */