#ifndef ___GEO_DATA_H___
#define ___GEO_DATA_H___

#include "Stdafx.h"

class CGeoLocation
{
public:
	CGeoLocation();
	CGeoLocation(const CGeoLocation& loc);
	CGeoLocation& operator=(const CGeoLocation& loc);

	int GetX() const {return m_X;}
	int GetY() const {return m_Y;}
	int GetLattitude() const {return m_Lattitude;}
	int GetLongitude() const {return m_Longitude;}
	void SetX(float val) {m_X = val;}
	void SetY(float val) {m_Y = val;}
	void SetLattitude(float val) {m_Lattitude = val;}
	void SetLongitude(float val) {m_Longitude = val;}

protected:
	int m_X;
	int m_Y;
	float m_Lattitude;
	float m_Longitude;
};


class CGeoData
{
public:
	CGeoData();
	CGeoData(const CGeoData& t);
	CGeoData& operator = (const CGeoData& t);

	CGeoLocation GetLocation() const {return m_Location;}
	CGeoLocation GetDepth() const {return m_Depth;}
	void SetDepth(real depth) {m_Depth = depth;}
	real GetRRS(int i) const {
		assert(i>=0 && i<m_vRRS.size());
		return m_vRRS[i];
	}
	void SetRRS(int i, real val) {
		assert(i>=0 && i<m_vRRS.size());
		m_vRRS[i] = val;
	}
	real GetIOP(int i) const {
		assert(i>=0 && i<m_IOPs.size());
		return m_IOPs[i];
	}
	void SetIOP(int i, real val) {
		assert(i>=0 && i<m_IOPs.size());
		m_IOPs[i] = val;
	}
	int GetTime() const {return m_Time;}
	int GetTransectID() const {return m_TransectID;}
	int GetID() const {return m_ID;}

	bool Read(ifstream& in);
	ofstream& Printf(ofstream& out);
protected:
	CGeoLocation		m_Location;
	vector<real>		m_vRRS;
	real				m_Depth;
	vector<real>		m_IOPs;
	int					m_Time;
	int					m_TransectID;
	int					m_ID;
};

#endif /* ___GEO_DATA_H___ */