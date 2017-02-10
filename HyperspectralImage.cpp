#include "Stdafx.h"
#include "HyperspectralImage.h"
#include "Utility.h"

#include <vector>
#include <fstream>
using namespace std;

CHyperspectralImage::CHyperspectralImage(): Image<real>()
{
	//m_Width=0;
	//m_Height=0;
	//m_Bands=0;
}

CHyperspectralImage::CHyperspectralImage(const CHyperspectralImage& image): Image<real>(image)
{
	//m_Filename = image.GetFilename();
	//m_Width = image.GetTotalWidth();
	//m_Height = image.GetTotalHeight();
	//m_Bands = image.GetNumSpectra();
	m_Parameters = image.GetParameters();
	m_vLambda = image.GetSpectra();
}

CHyperspectralImage&
CHyperspectralImage::operator =(const CHyperspectralImage& image)
{
	this->Image<real>::operator=(image);
	//m_Filename = image.GetFilename();
	//m_Width = image.GetTotalWidth();
	//m_Height = image.GetTotalHeight();
	//m_Bands = image.GetNumSpectra();
	m_Parameters = image.GetParameters();
	m_vLambda = image.GetSpectra();

	return *this;
}

CHyperspectralImage::CHyperspectralImage(const RealImage& image): Image<real>(image)
{
	//m_Filename.clear();
	//m_Width = image.NumCols();
	//m_Height = image.NumRows();
	//m_Bands = image.NumBands();
	//m_Parameters.Clear();
}

CHyperspectralImage&
CHyperspectralImage::operator =(const RealImage& image)
{
	this->Image<real>::operator=(image);
	/*ResizeImage(image.NumBands(), image.NumRows(), image.NumCols());
	for(int i=0; i<image.NumPixels(); ++i)
	{
		SetPixel(i, image.GetPixel(i));
	}*/
	//m_Filename.clear();
	//m_Width = image.NumCols();
	//m_Height = image.NumRows();
	//m_Bands = image.NumBands();
	//m_Parameters.Clear();

	return *this;
}

vector<real> 
CHyperspectralImage::GetBands(int r, int c) const
{
	vector<real> vData(NumBands());
	for(int k=0; k<NumBands(); ++k)
	{
		vData[k] = GetPixel(k, r, c);
	}
	return vData;
}

bool 
CHyperspectralImage::SetBands(const vector<real>& vdata, int r, int c) 
{
	if(r<0 || r>=NumRows() && c<0 && c>=NumCols())
	{
		return false;
	}
	if(vdata.size() < NumBands())
	{
		return false;
	}

	for(int k=0; k<NumBands(); ++k)
	{
		SetPixel(k, r, c, vdata[k]);
	}
	return true;
}


