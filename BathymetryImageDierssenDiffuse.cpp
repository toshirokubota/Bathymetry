// Bathymetry.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DataInterface.h"
#include "DierssenDiffuseMethod.h"
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
	const char* transectFile = inarg.Get("-ts", chTransectFile);
	const char* imageFile = inarg.Get("-i", chImage2);
	const char* paramFile = inarg.Get("-p", (char*) 0);
	const char* outputFile = inarg.Get("-o", "result.txt");
	const char* comments = inarg.Get("-cm", "Comments: ");
	int patchSize = inarg.Get("-sz", 5);
	int numiter = inarg.Get("-ni", 5);
	real alpha = inarg.Get("-alpha", 0.01);
	CDataInterface transect;
	transect.OpenData(transectFile);
	CParamContainer param = transect.GetData().GetParameters();

	CDataInterface image;
	image.OpenData(imageFile);

	CDierssenDiffuseMethod method(paramFile, alpha, numiter);
	int i, j, k;
	ofstream out(outputFile, ios::out);
	bool first = true;
	int sub_width = patchSize;
	int sub_height = patchSize;
	int index = (sub_height/2) * sub_width + sub_width/2;
	char chDate[16];
	char chTime[16];
	_strdate(chDate);
	_strtime(chTime);

	out << "remote-sensing inversion\tDierssenImageDiffuse" << endl;
	out << 9 << "\t" << 3 + method.NumEstimates() << endl;
	out << comments << endl; //descirbe any specifics about this experiment
	out << chDate << "\t" << chTime << endl;
	out << "Image\t" << imageFile << endl;
	out << "Transect\t" << transectFile << endl;
	out << "Params\t" << (paramFile==0 ? "Default": paramFile) << endl;
	out << "Alpha\t" << alpha << endl;
	out << "NIter\t" << numiter << endl;
	out << "Width/Height\t" << patchSize << endl;
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
			if(x != fNaN && y != fNaN)
			{
				if(image.ReadData((int)x, (int)y, sub_width, sub_height))
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
					method.PostProcess(); //rescale and clean up
					int i2, j2;
					int h = method.GetData().NumRows();
					int w = method.GetData().NumCols();
					out << x << "\t" << y << "\t";
					//for(index=0; index<sub_width*sub_height; ++index)//FOR_DEBUG
					//{//FOR_DEBUG
					for(j2=0; j2<method.NumEstimates(); ++j2)
					{
						if(bFailed)
						{
							out << "NaN" << "\t";
						}
						else
						{
							out << method.GetEstimate(method.GetEstimateName(j2), index) << "\t";
						}
					}
					out << method.ReadTimer() << "\t";
					out << endl;
					//}//FOR_DEBUG
				}
			}
		}
	}
	cout << numFailed << " points have failed." << endl;
	out.close();

	return 0;
}

