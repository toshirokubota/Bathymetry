#include "DierssenMethod.h"
#include "Utility.h"

const char* strDierssenEstimateNames[] = {"RRS555", "RRS670", "Depth"};
const char* strDierssenParameterNames[] = {"c1", "c2", "c3", "m", "b"};
//const real defaultDierssenParameters[] = {-0.1706, 0.8913, -0.2316, 2.67, -2.77};
const real defaultDierssenParameters[] = {-0.1706, 0.8913, -0.2316, 1.1917, -0.3166};

CDierssenMethod::CDierssenMethod(const char* paramFilename): CAbstractMethod()
{
	int m = sizeof(strDierssenParameterNames) / sizeof(strDierssenParameterNames[0]);
	int i;
	for(i=0; i<m; ++i)
	{
		m_Params.AddName(strDierssenParameterNames[i]);
	}
	bool bSuccess = false;
	if(paramFilename)
	{
		cout << "CDierssenMethod:: getting parameters from " << paramFilename << endl;
		ifstream in(paramFilename);
		if(!in.fail())
		{
			real val;
			bSuccess = true;
			for(i=0; i<m; ++i)
			{
				in >> val;
				if(in.good())
				{
					m_Params.AddValue(strDierssenParameterNames[i], val);
				}
				else
				{
					bSuccess = false;
					cerr << "CDierssenMethod:: Failed to read parameters from " << paramFilename << endl;
					cerr << "CDierssenMethod:: Cannot continue." << endl;
					abort();
				}
			}
		}
		else
		{
			cerr << "CDierssenMethod:: Failed to open " << paramFilename << " for read." << endl;
			cerr << "CDierssenMethod:: Cannot continue." << endl;
			abort();
		}
	}
	if(!bSuccess)
	{
		for(i=0; i<m; ++i)
		{
			m_Params.AddValue(strDierssenParameterNames[i], defaultDierssenParameters[i]);
		}
	}
	int n = sizeof(strDierssenEstimateNames) / sizeof(strDierssenEstimateNames[0]);
	for(i=0; i<n; ++i)
	{
		m_Estimate.AddName(strDierssenEstimateNames[i]);
	}
}

real
_computeDierssenDepth(real rs555, real rs670, real c1, real c2, real c3, real m, real b)
{
	real x = log10((double)rs555/rs670);
	//real z = exp(c1*x*x+c2*x+c3);
	x = x*m + b;
	real H = pow(10.0, c1*x*x+c2*x+c3);

	if(H < 0)
	{
		cerr << "Negative H found. " << H << endl;
	}
	return H;
}


bool
CDierssenMethod::Initialize()
{
	int m = NumEstimates(); 
	int i;
	for(int i=0; i<m; ++i)
	{
		m_Estimate.Resize(m_Estimate.GetName(i), m_Image.NumRows() * m_Image.NumCols());
	}

	int id555=0, id670=0;
	vector<real> lambda = m_Image.GetSpectra();
	for(int i=0; i<m_Image.NumBands(); ++i) 
	{
		if(Abs(lambda[i]-555.0)<Abs(lambda[id555]-555.0))
			id555=i;
		if(Abs(lambda[i]-670.0)<Abs(lambda[id670]-670.0))
			id670=i;
	}

	int j, k;
	for(i=0, k=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j < m_Image.NumCols(); ++j, ++k)
		{
			real rrs555 = m_Image.GetPixel(id555, i, j);
			real rrs670 = m_Image.GetPixel(id670, i, j);
			m_Estimate.SetParameter(strDierssenEstimateNames[0], k, rrs555);
			m_Estimate.SetParameter(strDierssenEstimateNames[1], k, rrs670);
			m_Estimate.SetParameter(strDierssenEstimateNames[2], k, 0);
		}
	}
	return true;
}

bool
CDierssenMethod::PostProcess()
{
	return true;
}

bool
CDierssenMethod::DoInversion()
{
	real c1 = m_Params.GetParameter(0, 0);
	real c2 = m_Params.GetParameter(1, 0);
	real c3 = m_Params.GetParameter(2, 0);
	real m = m_Params.GetParameter(3, 0);
	real b = m_Params.GetParameter(4, 0);

	int i, j, n;
	for(i=0, n=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j < m_Image.NumCols(); ++j, ++n)
		{
			real rrs555 = m_Estimate.GetParameter(strDierssenEstimateNames[0], n);
			real rrs670 = m_Estimate.GetParameter(strDierssenEstimateNames[1], n);
			real H = _computeDierssenDepth(rrs555, rrs670, c1, c2, c3, m, b);
			m_Estimate.SetParameter(strDierssenEstimateNames[2], n, H);
		}
	}

	return true;
}
