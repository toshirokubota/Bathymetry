#ifndef ___IDL_DATA_READER_H___
#define ___IDL_DATA_READER_H___
#include "Stdafx.h"
#include <vector>
using namespace std;

template<class T>
class CIDLReader {
public:
	CIDLReader(int data_type)
	{
		m_DataType = data_type;
	}
	int size() const
	{
		switch (m_DataType)
		{
		case 0:
			return 0;
		case 1:
			return 1;
		case 2:
		case 12:
			return 2;
		case 3:
		case 4:
		case 13:
			return 4;
		case 5:
		case 14:
		case 15:
			return 8;
		default:
			return 0;
		}
	}
	//currently only IDL type 4 and 12 are implemented. Implementing other data types is straightfoward.
	T readItem(ifstream& in)
	{
		char buf[16];
		in.read(buf, size());
		if (m_DataType == 4)
		{
			float* f = (float*)(buf);
			return (T)(*f);
		}
		else if (m_DataType == 12)
		{
			unsigned short* s = (unsigned short*)(buf);
			return (T)(*s);
		}
		else
		{
			return (T)0;
		}
	}
	vector<T> readItems(ifstream& in, int n)
	{
		char* buf = new char[n*size()];
		in.read(buf, n*size());
		vector<T> values(n);
		for (int i = 0; i < n; ++i)
		{
			if (m_DataType == 4)
			{
				float* f = (float*)(buf + i*size());
				values[i] = (T)(*f);
			}
			else if (m_DataType == 12)
			{
				unsigned short* s = (unsigned short*)(buf + i*size());
				values[i] = (T)(*s);
			}
			else
			{
				values[i] = (T)0;
			}
		}
		delete [] buf;
		return values;
	}
private:
	int m_DataType;
};

#endif /* ___IDL_DATA_READER_H___ */