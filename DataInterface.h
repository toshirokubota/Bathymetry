#ifndef ___DATA_INTERFACE_H___
#define ___DATA_INTERFACE_H___

#include "stdafx.h"
#include "mytype.h"
#include "HyperspectralImage.h"

enum InternalDataType {InvalidType, ImageType, TransectType};

class CDataInterface
{
public:
	CDataInterface();
	CDataInterface(const CDataInterface& data);
	const CDataInterface& operator=(const CDataInterface& data);

	bool OpenData(const char* headerfile);
	bool ReadData(int x, int y, int width = 1, int height = 1, int bands = -1);
	const CHyperspectralImage& GetData() const {return m_Image;}
	CHyperspectralImage GetData(int row1, int col1, int row2=-1, int col2=-1) const;
	string GetFilename() const {return m_Filename;}
	int GetWidth() const {return m_Width;}
	int GetHeight() const {return m_Height;}
	int GetNumSpectra() const {return m_Bands;}
	InternalDataType GetDataType() const {return m_DataType;}

protected:
	bool _OpenENVI(const char* filename);
	bool _OpenTransect(const char* filename);
	bool _ReadColumn(int x, int y);
	bool _ReadROI(int x, int y, int width, int height);
	bool _ReadImage(int z);

	CHyperspectralImage	m_Image;	//image data
	string				m_Filename;
	int					m_Width;
	int					m_Height;
	int					m_Bands;
	InternalDataType	m_DataType;
};

#endif /* ___DATA_INTERFACE_H___ */