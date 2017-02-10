#include "LeeMethod.h"
#include "Utility.h"
#include "LevenbergMarquardt.h"
#include "gradient.h"

const char* strLeeEstimateNames[] = {"P", "G", "BP", "B", "H"};

CLeeMethod::CLeeMethod(const char* filename): CAbstractMethod()
{
	m_Params.AddName("lambda");
	m_Params.AddName("a0");
	m_Params.AddName("a1");
	m_Params.AddName("aw");
	m_Params.AddName("albedo");
	m_Params.AddName("bbw");

	if(filename)
	{
		m_ParameterFilename = filename;
		if(!_InitializeParameters())
		{
			cerr << "Failed to initialize the parameters." << endl;
			cerr << "Check the file " << filename << " for correctness." << endl;
		}
	}

	m_Estimate.AddName(strLeeEstimateNames[0]);
	m_Estimate.AddName(strLeeEstimateNames[1]);
	m_Estimate.AddName(strLeeEstimateNames[2]);
	m_Estimate.AddName(strLeeEstimateNames[3]);
	m_Estimate.AddName(strLeeEstimateNames[4]);

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
	int m = sizeof(strLeeEstimateNames) / sizeof(strLeeEstimateNames[0]);
	m_Estimate.Clear();
	m_Estimate = CParamContainer(m, n);
	int i, j, k;
	for(int i=0; i<m; ++i)
	{
		m_Estimate.SetName(i, strLeeEstimateNames[i]);
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
	if(m_Params.GetLength("a0Interp"))
	{
		m_Params.Clear("lambdaInterp");
		m_Params.Clear("a0Interp");
		m_Params.Clear("a1Interp");
		m_Params.Clear("awInterp");
		m_Params.Clear("albedoInterp");
		m_Params.Clear("bbwInterp");
	}
	int i, j;
	m_Params.AddName("lambdaInterp");
	m_Params.Resize("lambdaInterp", m_Image.NumBands());
	m_Params.AddName("a0Interp");
	m_Params.Resize("a0Interp", m_Image.NumBands());
	m_Params.AddName("a1Interp");
	m_Params.Resize("a1Interp", m_Image.NumBands());
	m_Params.AddName("awInterp");
	m_Params.Resize("awInterp", m_Image.NumBands());
	m_Params.AddName("albedoInterp");
	m_Params.Resize("albedoInterp", m_Image.NumBands());
	m_Params.AddName("bbwInterp");
	m_Params.Resize("bbwInterp", m_Image.NumBands());
	
	for(i=0; i<m_Image.NumBands(); ++i)
	{
		real lambda = m_Image.GetSpectra(i);
		real val;
		m_Params.SetParameter("lambdaInterp", i, lambda);
		val = m_Params.GetParameterInterpolate("a0", "lambda", lambda);
		m_Params.SetParameter("a0Interp", i, val);
		val = m_Params.GetParameterInterpolate("a1", "lambda", lambda);
		m_Params.SetParameter("a1Interp", i, val);
		val = m_Params.GetParameterInterpolate("aw", "lambda", lambda);
		m_Params.SetParameter("awInterp", i, val);
		val = m_Params.GetParameterInterpolate("albedo", "lambda", lambda);
		m_Params.SetParameter("albedoInterp", i, val);
		val = m_Params.GetParameterInterpolate("bbw", "lambda", lambda);
		m_Params.SetParameter("bbwInterp", i, val);
	}
	return true;
}

bool
CLeeMethod::Initialize()
{
	if(_InitializeEstimates())
	{
		if(_InterpolateParameters())
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

real
_computeRRS(real lamb, real a0, real a1, real aw, real alb, real bbw,
					   real P, real G, real BP, real B, real H)
{
	real at=aw+P*(a0+ a1 * log(P)) + G * exp(-0.015 * (lamb - 440));
	real bb =  BP * pow((double)(400 / lamb),0.018932636) + bbw; //BP * sqrt(400 / lamb); //+ bbw;
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

//Global variables for calling _computeError
int g_X, g_Y; //the current pixel location
CHyperspectralImage* g_pImage;
CParamContainer* g_pParams;

double
_computeError(const vector<double>& vx)
//CLeeMethod::_computeError(const vector<double>& vx)
{
	int i, j, k;
	double error1 = 0, error2 = 0;
	double sum1 = 0, sum2 = 0;
	real P = vx[0];
	real G = vx[1];
	real BP = vx[2];
	real B = vx[3];
	real H = vx[4];
	int cnt1 = 0, cnt2 = 0;
	for(k=0; k<g_pImage->NumBands(); ++k)
	{
		real lamb = g_pImage->GetSpectra(k);
		if(_SupportedWavelength(lamb))
		{
			real rrs = g_pImage->GetPixel(k, g_Y, g_X); //global...
			real a0 = g_pParams->GetParameter("a0Interp", k);
			real a1 = g_pParams->GetParameter("a1Interp", k);
			real aw = g_pParams->GetParameter("awInterp", k);
			real albedo = g_pParams->GetParameter("albedoInterp", k);
			real bbw = g_pParams->GetParameter("bbwInterp", k);
			real rrsE = _computeRRS(lamb, a0, a1, aw, albedo, bbw, P, G, BP, B, H);
			if(lamb <= 675)
			{
				error1 += (rrs - rrsE) * (rrs - rrsE);
				sum1 += rrs;
				cnt1++;
			}
			else
			{
				error2 += (rrs - rrsE) * (rrs - rrsE);
				sum2 += rrs;
				cnt2++;
			}
			//cout << k << ": " << lamb << ", " << rrs << ", " << error << ", ";
			//cout << a0 << ", " << a1 << ", " << aw << ", " << albedo << ", " << bbw << ", " << rrsE << endl;
		}
	}
	double error = (error1/cnt1) + (error2/cnt2);
	error = sqrt(error) / (sum1 + sum2);
	cout << "Error = " << error << endl;
	return error;
}

bool
CLeeMethod::DoInversion()
{
	g_pImage = &m_Image;
	g_pParams = &m_Params;
	vector<double> vx(m_Estimate.NumParams());
	int i, j;
	for(i=0; i<this->m_Image.NumRows(); ++i)
	{
		for(j=0; j<this->m_Image.NumCols(); ++j)
		{
			vx[0] = m_Estimate.GetParameter(0, i*m_Image.NumCols()+j);
			vx[1] = m_Estimate.GetParameter(1, i*m_Image.NumCols()+j);
			vx[2] = m_Estimate.GetParameter(2, i*m_Image.NumCols()+j);
			vx[3] = m_Estimate.GetParameter(3, i*m_Image.NumCols()+j);
			vx[4] = m_Estimate.GetParameter(4, i*m_Image.NumCols()+j);
			g_X = j; 
			g_Y = i;
			vector<double> vres = Maths::Optimization::gradient(_computeError, vx, 1, 1.0e-5, 10);
			m_Estimate.SetParameter(0, i*m_Image.NumCols()+j, vres[0]);
			m_Estimate.SetParameter(1, i*m_Image.NumCols()+j, vres[1]);
			m_Estimate.SetParameter(2, i*m_Image.NumCols()+j, vres[2]);
			m_Estimate.SetParameter(3, i*m_Image.NumCols()+j, vres[3]);
			m_Estimate.SetParameter(4, i*m_Image.NumCols()+j, vres[4]);
		}
	}
	return true;
}
