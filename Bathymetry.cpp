// Bathymetry.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DataInterface.h"
#include "DierssenMethod.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CDataInterface input;

	input.OpenData("C:\\Kubota\\Research\\HyperSpectralImages\\images\\CombinedInSituTransects41Bands.txt");
	CDierssenMethod method;

	method.SetData(input.GetData());
	method.Initialize();
	method.DoInversion();

	vector<real> vDepth = method.GetEstimate(0);
	for(int i=0; i<vDepth.size(); ++i)
	{
		cout << vDepth[i] << endl;
	}

	return 0;
}

