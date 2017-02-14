// BathymetryDierssen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DataInterface.h"
#include "DierssenMethod.h"
#include "InputArguments.h"
#include <time.h>

const char chImage[] = "C:\\Kubota\\Research\\HyperSpectralImages\\images\\CombinedInSituTransects41Bands.txt";

int _tmain(int argc, _TCHAR* argv[])
{
	CInputArguments inarg(argc, argv);
	const char* imageHeaderFile = inarg.Get("-h", chImage);
	const char* imageDataFile = inarg.Get("-d", (char*)0);
	const char* paramFile = inarg.Get("-p", (char*)0);
	const char* outputFile = inarg.Get("-o", "result.txt");
	const char* comments = inarg.Get("-cm", "Comments: ");

	CDataInterface image;
	image.OpenData(imageHeaderFile, imageDataFile);
	CParamContainer param = image.GetData().GetParameters();
	CDierssenMethod method(paramFile);

	method.SetData(image.GetData());
	method.Initialize();

	char chDate[16];
	char chTime[16];
	_strdate(chDate);
	_strtime(chTime);

	ofstream out(outputFile, ios::out);
	out << "remote-sensing inversion\tDierssen" << endl;
	out << 5 << "\t" << 2 + method.NumEstimates() << endl;
	out << comments << endl; //descirbe any specifics about this experiment
	out << chDate << "\t" << chTime << endl;
	out << "Transect\t" << imageHeaderFile << endl;
	out << "Params\t" << (paramFile == 0 ? "Default": paramFile) << endl;
	out << "X" << "\t" << "Y" << "\t";

	int i, j, k;
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
			if(x == x && y == y)
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

