#include "LeeMethod.h"
#include "Utility.h"
#include "LevenbergMarquardt.h"

const char* strLeeEstimateNames[] = {"P", "G", "BP", "B", "H"};
const char* strLeeParameterNames[] = {"lambda", "a0", "a1", "aw", "albedo", "bbw"};
const char* strLeeInterpParameterNames[] = {"lambda_ip", "a0_ip", "a1_ip", "aw_ip", "albedo_ip", "bbw_ip"};

CLeeMethod::CLeeMethod(const char* filename): CAbstractMethod()
{
	int i, m;
	m = sizeof(strLeeParameterNames) / sizeof(strLeeParameterNames[0]);
	for(i=0; i<m; ++i)
	{
		m_Params.AddName(strLeeParameterNames[i]);
		m_InterpParam.AddName(strLeeInterpParameterNames[i]);
	}

	if(filename)
	{
		m_ParameterFilename = filename;
		if(!_InitializeParameters())
		{
			cerr << "Failed to initialize the parameters." << endl;
			cerr << "Check the file " << filename << " for correctness." << endl;
		}
	}

	m = sizeof(strLeeEstimateNames) / sizeof(strLeeEstimateNames[0]);
	for(i=0; i<m; ++i)
	{
		m_Estimate.AddName(strLeeEstimateNames[i]);
	}

	m_NumIter = 10;
	m_pLowerBounds = 0;
	m_pUpperBounds = 0;
	m_pWeights = 0;
#ifdef SCALE_MODEL
	//cout << "SCALE_MODEL option is on." << endl;
#endif
}

bool
CLeeMethod::LoadParameters(const char* filename)
{
	m_ParameterFilename = filename;
	if(!_InitializeParameters())
	{
		cerr << "Failed to initialize the parameters." << endl;
		cerr << "Check the file " << filename << " for correctness." << endl;
		return false;
	}
	else
	{
		return true;
	}
}

bool
CLeeMethod::_InitializeParameters()
{
	ifstream in(m_ParameterFilename.c_str(),ios::in);
	if (in.fail())
	{
		cerr << "CLeeMethod::_InitializeParameters:: Failed to open " << m_ParameterFilename << " for read.\n";
		cerr << "CLeeMethod::_InitializeParameters:: Cannot continue...\n";
		return false;
	}

	int numBands, numFields;
	in >> numBands >> numFields;
	vector<string> fieldNames(numFields);
	int i, j;
	for(i=0; i<numFields; ++i)
	{
		in >> fieldNames[i];
		if(in.bad())
		{
			cerr << "CLeeMethod::_InitializeParameters:: Read failure.\n";
			in.close();
			return false;
		}
		if(m_Params.GetLength(fieldNames[i]) < 0)
		{
			cerr << "CLeeMethod::_InitializeParameters:: Illegal format." << endl;
			cerr << "Unknown parameter " << fieldNames[i] << "." << endl;
			in.close();
			return false;
		}
	}
	for(i=0; i<numFields; ++i)
	{
		m_Params.Resize(fieldNames[i], numBands);
	}
	for(i=0; i<numBands; ++i)
	{
		for(j=0; j<numFields; ++j)
		{
			real val;
			in >> val;
			m_Params.SetParameter(fieldNames[j], i, val);
		}
	}
	return true;
}

bool
CLeeMethod::_InitializeEstimates()
{
	vector<real> lambda = m_Image.GetSpectra();
	int id440=0, id550=0, id640=0;
	for(int i=0; i<lambda.size(); ++i) {
		if(Abs(lambda[i]-440.0)<Abs(lambda[id440]-440.0))
			id440=i;
		if(Abs(lambda[i]-550.0)<Abs(lambda[id550]-550.0))
			id550=i;
		if(Abs(lambda[i]-640.0)<Abs(lambda[id640]-640.0))
			id640=i;
	}

	int n = m_Image.NumRows() * m_Image.NumCols();
	int m = this->NumEstimates(); 
	int i, j, k;
	for(int i=0; i<m; ++i)
	{
		m_Estimate.Resize(m_Estimate.GetName(i), n);
	}

	for(i=0, k=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j<m_Image.NumCols(); ++j, ++k)
		{
			real rrs440 = m_Image.GetPixel(id440, i, j);
			real rrs550 = m_Image.GetPixel(id550, i, j);
			real rrs640 = m_Image.GetPixel(id640, i, j);
			real P=0.1*pow((double)rrs440/rrs550,1.5);
			real G=1.5*P;
			real BP=5.*rrs640;
			real B=0.1;
			real H=3.*(1+rrs440/rrs550);
			P = Max(Lee_lower_bound[0], Min(Lee_upper_bound[0], P));
			G = Max(Lee_lower_bound[1], Min(Lee_upper_bound[1], G));
			BP = Max(Lee_lower_bound[2], Min(Lee_upper_bound[2], BP));
			B = Max(Lee_lower_bound[3], Min(Lee_upper_bound[3], B));
			H = Max(Lee_lower_bound[4], Min(Lee_upper_bound[4], H));

			m_Estimate.SetParameter(strLeeEstimateNames[0], k, P);
			m_Estimate.SetParameter(strLeeEstimateNames[1], k, G);
			m_Estimate.SetParameter(strLeeEstimateNames[2], k, BP);
			m_Estimate.SetParameter(strLeeEstimateNames[3], k, B);
			m_Estimate.SetParameter(strLeeEstimateNames[4], k, H);
		}
	}

	return true;
}

bool
CLeeMethod::_InterpolateParameters()
{
	int i, j;
	int m = sizeof(strLeeInterpParameterNames) / sizeof(strLeeInterpParameterNames[0]);
	for(i=0; i<m; ++i)
	{
		m_InterpParam.Resize(strLeeInterpParameterNames[i], m_Image.NumBands());
	}

	for(i=0; i<m_Image.NumBands(); ++i)
	{
		real lambda = m_Image.GetSpectra(i);
		for(j=0; j<m; ++j)
		{
			real val = m_Params.GetParameterInterpolate(strLeeParameterNames[j], "lambda", lambda);
			m_InterpParam.SetParameter(strLeeInterpParameterNames[j], i, val);
		}
	}
	return true;
}

bool 
CLeeMethod::_ScaleModels()
{
	int i, j, k;
	for(i=0, k=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j<m_Image.NumCols(); ++j, ++k)
		{
			real P = m_Estimate.GetParameter(strLeeEstimateNames[0], k);
			real G = m_Estimate.GetParameter(strLeeEstimateNames[1], k);
			real BP = m_Estimate.GetParameter(strLeeEstimateNames[2], k);
			real B = m_Estimate.GetParameter(strLeeEstimateNames[3], k);
			real H = m_Estimate.GetParameter(strLeeEstimateNames[4], k);

			P = (P - fLeeScaleOffset[0]) / fLeeScaleWidth[0];
			G = (G - fLeeScaleOffset[1]) / (fLeeScaleWidth[1]);
			BP = (BP - fLeeScaleOffset[2]) / (fLeeScaleWidth[2]);
			B = (B - fLeeScaleOffset[3]) / (fLeeScaleWidth[3]);
			H = (H - fLeeScaleOffset[4]) / (fLeeScaleWidth[4]);

			m_Estimate.SetParameter(strLeeEstimateNames[0], k, P);
			m_Estimate.SetParameter(strLeeEstimateNames[1], k, G);
			m_Estimate.SetParameter(strLeeEstimateNames[2], k, BP);
			m_Estimate.SetParameter(strLeeEstimateNames[3], k, B);
			m_Estimate.SetParameter(strLeeEstimateNames[4], k, H);
		}
	}
	return true;
}

bool 
CLeeMethod::_ReScaleModels()
{
	int i, j, k;
	for(i=0, k=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j<m_Image.NumCols(); ++j, ++k)
		{
			real P = m_Estimate.GetParameter(strLeeEstimateNames[0], k);
			real G = m_Estimate.GetParameter(strLeeEstimateNames[1], k);
			real BP = m_Estimate.GetParameter(strLeeEstimateNames[2], k);
			real B = m_Estimate.GetParameter(strLeeEstimateNames[3], k);
			real H = m_Estimate.GetParameter(strLeeEstimateNames[4], k);

			//scale back
			P = P * (fLeeScaleWidth[0]) + fLeeScaleOffset[0];
			G = G * (fLeeScaleWidth[1]) + fLeeScaleOffset[1];
			BP = BP * (fLeeScaleWidth[2]) + fLeeScaleOffset[2];
			B = B * (fLeeScaleWidth[3]) + fLeeScaleOffset[3];
			H = H * (fLeeScaleWidth[4]) + fLeeScaleOffset[4];

			m_Estimate.SetParameter(strLeeEstimateNames[0], k, P);
			m_Estimate.SetParameter(strLeeEstimateNames[1], k, G);
			m_Estimate.SetParameter(strLeeEstimateNames[2], k, BP);
			m_Estimate.SetParameter(strLeeEstimateNames[3], k, B);
			m_Estimate.SetParameter(strLeeEstimateNames[4], k, H);
		}
	}
	return true;
}

bool 
CLeeMethod::_SetBounds()
{
	int i;
	int l = m_Estimate.NumParams();
	if(m_pLowerBounds)
	{
		delete [] m_pLowerBounds;
		m_pLowerBounds = 0;
	}
	if(m_pUpperBounds)
	{
		delete [] m_pUpperBounds;
		m_pUpperBounds = 0;
	}
	m_pLowerBounds = new real[l];
	if(m_pLowerBounds == 0)
	{
		return false;
	}
	m_pUpperBounds = new real[l];
	if(m_pUpperBounds == 0)
	{
		delete [] m_pLowerBounds;
		m_pLowerBounds = 0;
		return false;
	}

	for(i=0; i<l; ++i)
	{
		m_pLowerBounds[i] = (Lee_lower_bound[i] - fLeeScaleOffset[i]) / fLeeScaleWidth[i];
		m_pUpperBounds[i] = (Lee_upper_bound[i] - fLeeScaleOffset[i]) / fLeeScaleWidth[i];
	}
	return true;
}

bool 
CLeeMethod::_SetErrorWeights()
{
	int n = m_Image.NumBands();
	if(m_pWeights)
	{
		delete [] m_pWeights;
		m_pWeights = 0;
	}
	m_pWeights = new real[n];
	if(m_pWeights == 0)
	{
		return false;
	}
	for(int i=0; i<n; ++i)
	{
		if(_SupportedWavelength(m_Image.GetSpectra(i)))
		{
			m_pWeights[i] = 1.0;
		}
		else
		{
			m_pWeights[i] = 0.0;
		}
	}

	return true;
}


bool
CLeeMethod::Initialize()
{
	if(_InitializeEstimates() && _InterpolateParameters())
	{
		if(_ScaleModels() && _SetBounds() && _SetErrorWeights())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool
CLeeMethod::PostProcess()
{
	_ReScaleModels();
	if(m_pWeights)
	{
		delete [] m_pWeights;
		m_pWeights = 0;
	}
	if(m_pLowerBounds)
	{
		delete [] m_pLowerBounds;
		m_pLowerBounds = 0;
	}
	if(m_pUpperBounds)
	{
		delete [] m_pUpperBounds;
		m_pUpperBounds = 0;
	}
	return true;
}

real
_computeRRS_Lee(const vector<real>& vmodel, int n, const void* p)
{
	CParamContainer* pParam = (CParamContainer *) p;
	real lamb = pParam->GetParameter(0,n);
	real a0  = pParam->GetParameter(1,n);
	real a1 = pParam->GetParameter(2,n);
	real aw = pParam->GetParameter(3,n);
	real alb = pParam->GetParameter(4,n);
	real bbw = pParam->GetParameter(5,n);
	real sP = vmodel[0];
	real sG = vmodel[1];
	real sBP = vmodel[2];
	real sB = vmodel[3];
	real sH = vmodel[4];

	//scale back
	real P = sP * (fLeeScaleWidth[0]) + fLeeScaleOffset[0];
	real G = sG * (fLeeScaleWidth[1]) + fLeeScaleOffset[1];
	real BP = sBP * (fLeeScaleWidth[2]) + fLeeScaleOffset[2];
	real B = sB * (fLeeScaleWidth[3]) + fLeeScaleOffset[3];
	real H = sH * (fLeeScaleWidth[4]) + fLeeScaleOffset[4];

	real at=aw+P*(a0+ a1 * log(P)) + G * exp(-0.015 * (lamb - 440));
	real bb =  BP * pow((double)(400 / lamb),0.018932636) + bbw; 
	real u = bb / (at + bb); 
	real karpa = at + bb;

	real Duc = 1.03 * sqrt(1 + 2.4 * u);
	real Dub = 1.05 * sqrt(1 + 5.5 * u);

	real rrs_c = (0.084 + 0.17 * u) * u * (1 - exp(-(1.2 + Duc) * karpa * H));
	real rrs_b = 0.3183 * B * alb * exp(-(1.2 + Dub) * karpa * H);

	real term1 = rrs_c + rrs_b;
	real term2 = 0.52 * term1 / (1 - 1.6 * term1);

	return term2;
}

surreal
_computeRRSD_Lee(const vector<surreal>& vmodel, int n, const void* p)
{
	CParamContainer* pParam = (CParamContainer *) p;
	surreal lamb = pParam->GetParameter(0,n);
	surreal a0  = pParam->GetParameter(1,n);
	surreal a1 = pParam->GetParameter(2,n);
	surreal aw = pParam->GetParameter(3,n);
	surreal alb = pParam->GetParameter(4,n);
	surreal bbw = pParam->GetParameter(5,n);
	surreal sP = vmodel[0];
	surreal sG = vmodel[1];
	surreal sBP = vmodel[2];
	surreal sB = vmodel[3];
	surreal sH = vmodel[4];

	//scale back
	surreal P = sP * (fLeeScaleWidth[0]) + fLeeScaleOffset[0];
	surreal G = sG * (fLeeScaleWidth[1]) + fLeeScaleOffset[1];
	surreal BP = sBP * (fLeeScaleWidth[2]) + fLeeScaleOffset[2];
	surreal B = sB * (fLeeScaleWidth[3]) + fLeeScaleOffset[3];
	surreal H = sH * (fLeeScaleWidth[4]) + fLeeScaleOffset[4];

	surreal at=aw+P*(a0+ a1 * log(P)) + G * exp(-0.015 * (lamb - 440));
	surreal bb =  BP * pow((double)(400 / lamb),0.018932636) + bbw; //BP * sqrt(400 / lamb); //+ bbw;
	surreal u = bb / (at + bb); 
	surreal karpa = at + bb;

	surreal Duc = 1.03 * sqrt(1 + 2.4 * u);
	surreal Dub = 1.05 * sqrt(1 + 5.5 * u);

	surreal rrs_c = (0.084 + 0.17 * u) * u * (1 - exp(-(1.2 + Duc) * karpa * H));
	surreal rrs_b = 0.3183 * B * alb * exp(-(1.2 + Dub) * karpa * H);

	surreal term1 = rrs_c + rrs_b;
	surreal term2 = 0.52 * term1 / (1 - 1.6 * term1);

	return term2;
}

bool
CLeeMethod::DoInversion()
{
	int i, j, k;
	int n = m_Image.NumBands();
	int l = m_Estimate.NumParams();
	//ofstream out("logLee.txt", ios::out);

	vector<real> vRRS(n);
	vector<real> vEstimates(l);
	for(i=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j<m_Image.NumCols(); ++j)
		{
			real rrs_sum = 0;
			for(k=0; k<n; ++k)
			{
				vRRS[k] = m_Image.GetPixel(k,i,j);
				rrs_sum += vRRS[k] * vRRS[k];
			}
			/*for(k=0; k<n; ++k)
			{
				if(_SupportedWavelength(m_Image.GetSpectra(i)))
				{
					m_pWeights[i] = 1.0;// / rrs_sum;
				}
				else
				{
					m_pWeights[i] = 0.0;
				}
			}*/
			for(k=0; k<l; ++k)
			{
				vEstimates[k] = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j);
			}

			//out << i << ", " << j << endl;

			real rate = 0.001;
			int maxIter = 10;
			bool bRet = LevenbergMarquardt(vRRS, vEstimates,
				_computeRRS_Lee, _computeRRSD_Lee, 
				maxIter, rate,
				(void *) &m_InterpParam, 
				m_pLowerBounds, m_pUpperBounds,
				m_pWeights,
				0);
			if(!bRet)
			{
				cerr << "LevenbergMarquardt exception. at (" << i << ", " << j << ")\n";
				for(k=0; k<l; ++k)
				{
					m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, fNaN);
				}
				return false;
			}

			for(k=0; k<l; ++k)
			{
				m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, vEstimates[k]);
			}
		}
    }

	//out.close();

	return true;
}
