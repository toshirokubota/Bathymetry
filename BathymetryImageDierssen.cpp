// Bathymetry.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DataInterface.h"
#include "DierssenMethod.h"
#include "ParamInterface.h"
#include "InputArguments.h"
#include <time.h>

const char chImage1[] = "C:\\Kubota\\Research\\HyperSpectralImages\\images\\Run02Seq02calff_tafkaa_R_rs_ss5_GEOREF_IGM_spatial_subset_FLOAT_specresamp_mask_threshpt013_b33_716nm.hdr";
const char chImage2[] = "C:\\Kubota\\Research\\HyperSpectralImages\\images\\EMP_LINE_Tafkaa_station04_HTSRB_Super_Georef_Run02Seq02calff_41bands_corrected.hdr";
const char chImage3[] = "C:\\Kubota\\Research\\HyperSpectralImages\\images\\EMP_LINE_Tafkaa_station04_HTSRB_Super_Georef_Run02Seq02calff_41bands_10buncorrected.hdr";
//const char chTransectFile[] = "C:\\Kubota\Research\\HyperSpectralImages\images\\all_transects_SUPERGEOREF_PHILLS_NEW\\transectsAll.txt";
const char chTransectFile[] = "C:\\Kubota\\Research\\HyperSpectralImages\\images\\CombinedInSituTransects41Bands.txt";

int _tmain(int argc, _TCHAR* argv[])
{
	CInputArguments inarg(argc, argv);
	const char* transectFile = inarg.Get("-ts", (char*)0);
	const char* imageHeaderFile = inarg.Get("-h", chImage2);
	const char* imageDataFile = inarg.Get("-d", chImage2);
	const char* paramFile = inarg.Get("-p", (char*)0);
	const char* outputFile = inarg.Get("-o", "result.txt");
	const char* comments = inarg.Get("-cm", "Comments: ");
	if (transectFile == 0 || imageHeaderFile == 0)
	{
		cerr << "Missing transect file (-ts) and image header file (-h)." << endl;
		exit(-1);
	}
	if (imageDataFile == 0)
	{
		string filename(imageHeaderFile);
		string::size_type pos = filename.rfind(".");
		if (pos == string::npos)
		{
			imageHeaderFile = (filename + ".img").c_str();
		}
		else
		{
			imageHeaderFile = (filename.substr(0, pos) + ".img").c_str();
		}
	}
	CDataInterface transect;
	transect.OpenData(transectFile);
	CParamContainer param = transect.GetData().GetParameters();

	CDataInterface image;
	image.OpenData(imageHeaderFile, imageDataFile);
	CDierssenMethod method(paramFile);
	int i, j, k;
	ofstream out(outputFile, ios::out);
	bool first = true;
	char chDate[16];
	char chTime[16];
	_strdate(chDate);
	_strtime(chTime);

	out << "remote-sensing inversion\tDierssenImage" << endl;
	out << 6 << "\t" << 3 + method.NumEstimates() << endl;
	out << comments << endl; //descirbe any specifics about this experiment
	out << chDate << "\t" << chTime << endl;
	out << "ImageHeader\t" << imageHeaderFile << endl;
	out << "ImageData\t" << imageDataFile << endl;
	out << "Transect\t" << transectFile << endl;
	out << "Params\t" << (paramFile==0 ? "Default": paramFile) << endl;
	out << "X" << "\t" << "Y" << "\t";
	for(i=0; i<method.NumEstimates(); ++i)
	{
		out << method.GetEstimateName(i) << "\t";
	}
	out << "T" << "\t"; //timer
	out << endl;

	int numFailed = 0;
	for(i=0, k=0; i<transect.GetHeight(); ++i)
	{
		for(j=0; j<transect.GetWidth(); ++j, k++)
		{
			real x = param.GetParameter("X", k);
			real y = param.GetParameter("Y", k);
			cout << k << " (" << (int)x << ", " << (int)y << ")" << endl;
			if(x == x && y == y)
			{
				if(image.ReadData((int)x, (int)y, 1, 1)) 
				{
					method.SetData(image.GetData());
					method.SetTimer();
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
					method.PostProcess(); //re-scale and clean up
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
						out << method.ReadTimer() << "\t";
						out << endl;
					}
				}
			}
		}
	}
	cout << numFailed << " points have failed." << endl;
	out.close();

	return 0;
}

