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
	//CDataInterface(const CDataInterface& data);
	//const CDataInterface& operator=(const CDataInterface& data);

	bool OpenData(const char* headerfile, const char* datafile=NULL);
	bool ReadData(int x, int y, int width = 1, int height = 1, int bands = -1);
	const CHyperspectralImage& GetData() const {return m_Image;}
	CHyperspectralImage GetData(int row1, int col1, int row2=-1, int col2=-1) const;
	string GetFilename() const { return m_HeaderFilename; }
	string GetDataname() const { return m_DataFilename; }
	int GetWidth() const { return m_Width; }
	int GetHeight() const {return m_Height;}
	int GetNumSpectra() const {return m_Bands;}
	InternalDataType GetDataType() const {return m_DataType;}
	int GetIdlDataType() const { return m_IdlDataType; }

protected:
	bool _OpenENVI(const char* filename);
	bool _OpenTransect(const char* filename);
	bool _ReadColumn(int x, int y);
	bool _ReadROI(int x, int y, int width, int height);
	bool _ReadImage(int z);

	CHyperspectralImage	m_Image;	//image data
	string				m_HeaderFilename;
	string				m_DataFilename;
	int					m_Width;
	int					m_Height;
	int					m_Bands;
	InternalDataType	m_DataType;
	int					m_IdlDataType; //see http://www.harrisgeospatial.com/docs/IDL_Data_Types.html
};

#endif /* ___DATA_INTERFACE_H___ */