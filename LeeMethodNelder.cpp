#include "LeeMethod.h"
#include "Utility.h"
#include "LevenbergMarquardt.h"
#include "nelder.h"
#include "rndm.h"

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
CLeeMethod::Initialize()
{
	if(_InitializeEstimates() && _InterpolateParameters())
	{
		return true;
	}
	else
	{
		return false;
	}
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
	real P = vmodel[0];
	real G = vmodel[1];
	real BP = vmodel[2];
	real B = vmodel[3];
	real H = vmodel[4];

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
	surreal P = vmodel[0];
	surreal G = vmodel[1];
	surreal BP = vmodel[2];
	surreal B = vmodel[3];
	surreal H = vmodel[4];

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

vector<real> g_vRRS;
real* g_Weights;

real
_computeRRS_LeeNelder(double* pModel, const void* p)
{
	CParamContainer* pParam = (CParamContainer *) p;
	real P = pModel[0];
	real G = pModel[1];
	real BP = pModel[2];
	real B = pModel[3];
	real H = pModel[4];
	int numB = g_vRRS.size();
	real totalSum = 0;
	for(int n = 0; n<numB; ++n)
	{
		real lamb = pParam->GetParameter(0,n);
		real a0  = pParam->GetParameter(1,n);
		real a1 = pParam->GetParameter(2,n);
		real aw = pParam->GetParameter(3,n);
		real alb = pParam->GetParameter(4,n);
		real bbw = pParam->GetParameter(5,n);

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

		real error = term2 - g_vRRS[n];
		totalSum += g_Weights[n] * error * error;
		//cout << error << ", " << g_Weights[n] << ", " << g_vRRS[n] << endl;
	}

	//cout << "_computeRRS_LeeNelder: " << totalSum << endl;
	return totalSum;
}

bool
CLeeMethod::DoInversion()
{
	int i, j, k;
	int n = m_Image.NumBands();
	int l = m_Estimate.NumParams();
	real* weights = new real[n];
	for(i=0; i<n; ++i)
	{
		if(_SupportedWavelength(m_Image.GetSpectra(i)))
		{
			weights[i] = 1.0;
		}
		else
		{
			weights[i] = 0.0;
		}
	}
	vector<real> vRRS(n);
	vector<real> vEstimates(l);

	g_Weights = weights;

	double** pModels = new double*[l+1];
	for(i=0; i<l+1; ++i)
	{
		pModels[i] = new double[l];
	}

	for(i=0; i<m_Image.NumRows(); ++i)
	{
		for(j=0; j<m_Image.NumCols(); ++j)
		{
			for(k=0; k<n; ++k)
			{
				vRRS[k] = m_Image.GetPixel(k,i,j);
			}
			g_vRRS = vRRS;
			for(k=0; k<l; ++k)
			{
				vEstimates[k] = m_Estimate.GetParameter(k, i*m_Image.NumCols()+j);
				pModels[0][k] = vEstimates[k];
				for(int k2=1; k2<l+1; ++k2)
				{
					real mid = (Lee_upper_bound[k] + Lee_lower_bound[k]) / 2.0;
					pModels[k2][k] = mid + (rndm(0)-0.5) * (Lee_upper_bound[k] - Lee_lower_bound[k]);
				}
			}

			cout << i << "," << j << " Before : " << _computeRRS_LeeNelder(pModels[l], &m_InterpParam) << endl;
			real rate = 0.001;
			int maxIter = 10;
			/*bool bRet = LevenbergMarquardt(vRRS, vEstimates,
				_computeRRS_Lee, _computeRRSD_Lee, 
				maxIter, rate,
				(void *) &m_InterpParam, 
				Lee_lower_bound, Lee_upper_bound,
				weights);*/
			Maths::Optimization::nelder(_computeRRS_LeeNelder, l, pModels, 1E-10, 1000, &m_InterpParam);
			int bRet = true;
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
				//!!!!m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, vEstimates[k]);
				m_Estimate.SetParameter(k, i*m_Image.NumCols()+j, pModels[l][k]);
			}
			cout << i << "," << j << " After : " << _computeRRS_LeeNelder(pModels[l], &m_InterpParam) << endl;
		}
    }

	delete [] weights;
	weights = 0;
	for(i=0; i<l+1; ++i)
	{
		delete [] pModels[i];
	}
	delete [] pModels;

	return true;
}
