#include "DataInterface.h"
#include "Utility.h"
#include <string>
using namespace std;
#include "IdlDataReader.h"

CDataInterface::CDataInterface()
{
	m_Width=0;
	m_Height=0;
	m_Bands=0;
	m_DataType = InvalidType;
	m_IdlDataType = 0; //UNDEFINED.
};

/*CDataInterface::CDataInterface(const CDataInterface& data)
{
	m_Image = data.GetData();
	m_HeaderFilename = data.GetFilename();
	m_DataFilename = data.GetDataname();
	m_Width = data.GetWidth();
	m_Height = data.GetHeight();
	m_Bands = data.GetNumSpectra();
	m_DataType = data.GetDataType();
	m_IdlDataType = data.GetIdlDataType();
}

const CDataInterface&
CDataInterface:: operator=(const CDataInterface& data)
{
	m_Image = data.GetData();
	m_Filename = data.GetFilename();
	m_Width = data.GetWidth();
	m_Height = data.GetHeight();
	m_Bands = data.GetNumSpectra();
	m_DataType = data.GetDataType();

	return *this;
}*/

bool
CDataInterface::_OpenENVI(const char* filename)
{
	ifstream in(filename,ios::in);

	if (in.fail())
	{
		cerr << "CDataInterface::_OpenENVI:: Failed to open " << filename << " for read.\n";
		cerr << "CDataInterface::_OpenENVI:: Cannot continue...\n";
		return false;
	}
	
	string sbuf1, sbuf2;
	getline(in,sbuf1);  //ENVI
	if (sbuf1 != "ENVI") return false;
	//look for width, height, and bands
	m_Width = m_Height = m_Bands = 0;
	while (!in.eof() && in.good())
	{
		getline(in, sbuf1);
		if (sbuf1.find("samples") != string::npos)
		{
			string::size_type n = sbuf1.find("=");
			if (n != string::npos)
			{
				m_Width = atoi(sbuf1.substr(n + 1).c_str());
			}
		}
		else if (sbuf1.find("lines") != string::npos)
		{
			string::size_type n = sbuf1.find("=");
			if (n != string::npos)
			{
				m_Height = atoi(sbuf1.substr(n + 1).c_str());
			}
		}
		else if (sbuf1.find("bands") != string::npos)
		{
			string::size_type n = sbuf1.find("=");
			if (n != string::npos)
			{
				m_Bands = atoi(sbuf1.substr(n + 1).c_str());
			}
		}
		else if (sbuf1.find("data type") != string::npos)
		{
			string::size_type n = sbuf1.find("=");
			if (n != string::npos)
			{
				m_IdlDataType = atoi(sbuf1.substr(n + 1).c_str());
			}
		}
	}
	/*getline(in,sbuf1);	//	Applied Mask Result
	in >> sbuf1 >> sbuf2 >> m_Width;
	in >> sbuf1 >> sbuf2 >> m_Height;
	in >> sbuf1 >> sbuf2 >> m_Bands;*/
	if(in.bad() || m_Width==0 || m_Height==0 || m_Bands==0)
	{
		in.close();
		return false;
	}

	vector<real> vlambda(m_Bands);
	double val;
	while (!in.eof() && in.good()) 
	{
		char buffer[64];
		getline(in, sbuf1);  
		if(sbuf1.find("wavelength = {")!=-1) 
		{
			int k;
			for(k=0; k<m_Bands-1; ++k) 
			{
				in >> buffer; // >> ",";
				sscanf(buffer, "%lf,", &val);
				val*=1000.0;
				vlambda[k]=(real)val;
			}
			in >> buffer; // >> ",";
			sscanf(buffer, "%lf}", &val);
			val*=1000.0;
			vlambda[k]=val;
		}
		if (in.bad())
			break;
	}
	m_Image.SetSpectra(vlambda);

	in.close();
	return true;
}

bool
CDataInterface::_OpenTransect(const char* filename)
{
	ifstream in(filename,ios::in);

	if (in.fail())
	{
		cerr << "CDataInterface::_OpenTransect:: Failed to open " << filename << " for read.\n";
		cerr << "CDataInterface::_OpenTransect:: Cannot continue...\n";
		return false;
	}

	string sbuf;
	in >> sbuf;
	if (sbuf != "Transect")
	{
		cerr << "CDataInterface::_OpenTransect:: Illegal format.\n";
		return false;
	}

	int numData;
	int numHeads;
	int numBands;
	in >> numData >> numHeads >> numBands;

	vector<real> vlambda(numBands, fNaN);
	in >> sbuf; //Lambda
	int i, j;
	for(i=0; i<numBands; ++i)
	{
		in >> vlambda[i];
		vlambda[i] *= 1000;
		if(in.bad())
		{
			cerr << "CDataInterface::_OpenTransect:: Read failure.\n";
			in.close();
			return false;
		}
	}
	in.ignore(2056, '\n');

	CParamContainer params(numHeads, numData);
	vector<string> names(numHeads);
	for(j=0; j<numHeads; ++j)
	{
		in >> names[j];
		params.SetName(j, names[j]);
	}
	in.ignore(2056, '\n');

	RealImage tmpImage(numBands, 1, numData);
	for(i=0; i<numData; ++i)
	{
		for(j=0; j<numHeads; ++j)
		{
			real val;
			in >> val;
			params.SetParameter(names[j], i, val);
		}
		for(j=0; j<numBands; ++j)
		{
			real val;
			in >> val;
			tmpImage.SetPixel(j, 0, i, val);
		}
		if(in.bad())
		{
			cerr << "CDataInterface::_OpenTransect:: Read failure.\n";
			in.close();
			return false;
		}
	}

	m_Image = tmpImage;
	m_Image.SetSpectra(vlambda);
	m_Image.SetParameters(params);
	m_Width = numData;
	m_Height = 1;
	m_Bands = numBands;

	in.close();
	return true;
}

bool 
CDataInterface::OpenData(const char* filename, const char* datafile)
{
	string sbuf;
	m_HeaderFilename = filename;
	m_DataFilename = datafile == 0 ? "": datafile;
	m_IdlDataType = 0;
	ifstream in(m_HeaderFilename.c_str(), ios::in);
	if (in.fail())
	{
		cerr << "CDataInterface::OpenImage:: Failed to open " << m_HeaderFilename << " for read.\n";
		cerr << "CDataInterface::OpenImage:: Cannot continue...\n";
		return false;
	}

	in >> sbuf;
	in.close();
	if(sbuf == "ENVI")
	{
		m_DataType = ImageType;
		return _OpenENVI(m_HeaderFilename.c_str());
	}
	else if(sbuf == "Transect")
	{
		m_DataType = TransectType;
		return _OpenTransect(m_HeaderFilename.c_str());
	}
	else
	{
		cerr << "CDataInterface::OpenImage:: Unsupported format.\n";
		return false;
	}
}

bool CDataInterface::_ReadColumn(int x, int y)
{
	ifstream in(m_DataFilename.c_str(),ios::in | ios::binary);

	if (in.fail())
	{
		cerr << "CDataInterface::ReadColumn:: Failed to open " << m_DataFilename << " for read.\n";
		cerr << "CDataInterface::ReadColumn:: Cannot continue...\n";
		return false;
	}
	CIDLReader<real> reader(m_IdlDataType);

	int offset = (y * m_Width + x) * reader.size();
	int stride = m_Width * m_Height * reader.size();
	int i;
	vReal vdata(m_Bands);
	for(i=0; in.good() && i<m_Bands; ++i) {
		in.clear();
		in.seekg(i*stride + offset);
		float val = reader.readItem(in);
		vdata[i] = (real)val;
		//cout << "CDataInterface::_ReadColumn - " << vdata[i] << endl;
	}
	in.close();

	if(i < m_Bands)
	{
		cerr << "CDataInterface::ReadColumn:: Failed to read " << m_DataFilename << " for read.\n";
		cerr << "CDataInterface::ReadColumn:: Only " << i << " elements were read instead of " << m_Bands << endl;
		cerr << "CDataInterface::ReadColumn:: Cannot continue...\n";
		return false;
	}

	m_Image.ResizeImage(m_Bands, 1, 1);
	for(i=0; i<m_Bands; ++i)
	{
		m_Image.SetPixel(i, vdata[i]);
	}
	
	return true;
}


bool CDataInterface::_ReadROI(int x, int y, int width, int height)
{
	ifstream in(m_DataFilename.c_str(), ios::in | ios::binary);

	if (in.fail())
	{
		cerr << "CDataInterface::ReadROI:: Failed to open " << m_DataFilename << " for read.\n";
		cerr << "CDataInterface::ReadROI:: Cannot continue...\n";
		return false;
	}
	CIDLReader<real> reader(m_IdlDataType);

	int bx = x - width/2;
	int ex = bx + width;
	int by = y - height/2;
	int ey = by + height;
	bx = Max(0, bx);
	ex = Min(m_Width, ex);
	by = Max(0, by);
	ey = Min(m_Height, ey);
	width = ex - bx;
	height = ey - by;

	//int offset = (m_Width * by + bx) * sizeof(float);
	//int stride1 = (m_Width - width) * sizeof(float);
	//int stride2 = (m_Width * m_Height - height * m_Width - width) * sizeof(float);
	//in.seekg(offset);
	RealImage tmpImage(m_Bands, height, width);
	//float* pdata = new float [width];
	//char* buffer1 = new char[stride1];
	//char* buffer2 = new char[stride2];
	int i, j, k;
	for(i=0; i<m_Bands; ++i)
	{
		for(j=0; j<height; ++j)
		{
			int offset = i * m_Width * m_Height + (j+by) * m_Width + bx;
			in.seekg(offset * sizeof(float));
			vector<real> values = reader.readItems(in, width);
			if (in.fail())
			{
				cerr << "CDataInterface::ReadImage:: Failed to read data from " << m_DataFilename << endl;
				cerr << "CDataInterface::ReadImage:: Cannot continue...\n";
				in.close();
				return false;
			}
			for(k=0; k<width; ++k)
			{
				tmpImage.SetPixel(i, j, k, values[k]);
			}
		}
	}
	in.close();
	//delete [] buffer1;
	//delete [] buffer2;

	m_Image.ResizeImage(tmpImage.NumBands(), tmpImage.NumRows(), tmpImage.NumCols());
	for(i=0; i<tmpImage.NumPixels(); ++i)
	{
		m_Image.SetPixel(i, tmpImage.GetPixel(i));
	}

	return true;
}

bool CDataInterface::_ReadImage(int z)
{
	ifstream in(m_DataFilename.c_str(),ios::in | ios::binary);

	if (in.fail())
	{
		cerr << "CDataInterface::ReadImage:: Failed to open " << m_DataFilename << " for read.\n";
		cerr << "CDataInterface::ReadImage:: Cannot continue...\n";
		return false;
	}
	CIDLReader<real> reader(m_IdlDataType);
	int offset = (m_Width * m_Height * z) * reader.size();
	in.seekg(offset);
	int i;
	float* pdata = new float [m_Width * m_Height];
	vector<real> values = reader.readItems(in, m_Width * m_Height);
	if (in.fail())
	{
		cerr << "CDataInterface::ReadImage:: Failed to read data from " << m_DataFilename << endl;
		cerr << "CDataInterface::ReadImage:: Cannot continue...\n";
		in.close();
		return false;
	}
	in.close();

	m_Image.ResizeImage(1, m_Width, m_Height);
	for(i=0; i<m_Width * m_Height; ++i)
	{
		m_Image.SetPixel(i, values[i]);
	}
	delete [] pdata;
	pdata = 0;

	return true;
}

bool 
CDataInterface::ReadData(int x, int y, int width, int height, int band)
{
	if(width>1 && height>=1 || width>=1 && height>1)
	{
		return _ReadROI(x, y, width, height);
	}
	else if(width == 1 && height == 1)
	{
		return _ReadColumn(x, y);
	}
	else if(band >= 0 && band < m_Bands)
	{
		return _ReadImage(band);
	}
	else
	{
		cerr << "CDataInterface::ReadData:: Cannot resolve the parameters." << endl;
		return false;
	}
}

CHyperspectralImage 
CDataInterface::GetData(int row1, int col1, int row2, int col2) const
{
	if(row2<0)
	{
		row2 = row1;
	}
	if(col2<0)
	{
		col2=col1;
	}

	int brow = Min(row1, row2);
	int bcol = Min(col1, col2);
	int erow = Max(row1, row2);
	int ecol = Max(col1, col2);

	CHyperspectralImage image = m_Image.ExtractROI(0, m_Image.NumBands()-1, brow, erow, bcol, ecol);
	image.SetSpectra(m_Image.GetSpectra());

	return image;
}
