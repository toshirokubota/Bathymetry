// Bathymetry.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DataInterface.h"
#include "LeeMethod.h"
#include "InputArguments.h"
#include <time.h>

int _tmain(int argc, _TCHAR* argv[])
{
	CInputArguments inarg(argc, argv);
	const char* imageFile = inarg.Get("-i", "C:\\Kubota\\Research\\HyperSpectralImages\\images\\CombinedInSituTransects41Bands.txt");
	const char* paramFile = inarg.Get("-p", "C:\\Kubota\\Research\\HyperSpectralImages\\truth\\Lee_model_parameters_0306_2007.txt");
	const char* outputFile = inarg.Get("-o", "result.txt");
	CDataInterface image;
	image.OpenData(imageFile);
	CParamContainer param = image.GetData().GetParameters();
	CLeeMethod method(paramFile);
	int i, j, k;
	char chDate[16];
	char chTime[16];
	_strdate(chDate);
	_strtime(chTime);

	ofstream out(outputFile, ios::out);
	out << "remote-sensing inversion\tLee" << endl;
	out << 4 << "\t" << 2 + method.NumEstimates() << endl;
	out << chDate << "\t" << chTime << endl;
	out << "Transect\t" << imageFile << endl;
	//out << "Transect\t" << transectFile << endl;
	out << "Params\t" << paramFile << endl;
	out << "X" << "\t" << "Y" << "\t";
	for(i=0; i<method.NumEstimates(); ++i)
	{
		out << method.GetEstimateName(i) << "\t";
	}
	out << endl;

	int numFailed = 0;
	for(i=0, k=0; i<image.GetHeight(); ++i)
	{
		for(j=0; j<image.GetWidth(); ++j, k++)
		{
			real x = param.GetParameter("X", k);
			real y = param.GetParameter("Y", k);
			cout << k << " (" << (int)x << ", " << (int)y << ")" << endl;
			if (x == x && y == y)
			{
				method.SetData(image.GetData(i,j));
				method.Initialize();
				bool bFailed = true;
				if(method.DoInversion())
				{
					bFailed = false;
				}
				else
				{
					numFailed++;
				}
				method.PostProcess();
				int i2, j2;
				int h = method.GetData().NumRows();
				int w = method.GetData().NumCols();
				out << x << "\t" << y << "\t";
				for(i2=0; i2<w*h; ++i2)
				{
					for(j2=0; j2<method.NumEstimates(); ++j2)
					{
						if(bFailed)
						{
							out << "NaN" << "\t";
						}
						else
						{
							out << method.GetEstimate(method.GetEstimateName(j2), i2) << "\t";
						}
					}
					out << endl;
				}
			}
		}
	}
	cout << numFailed << " points have failed." << endl;
	out.close();
	return 0;
}

