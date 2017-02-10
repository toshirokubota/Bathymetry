#include "StdAfx.h"
#include "Utility.h"

bool _SupportedWavelength(real r)
{
	if (r >= 400 && r <= 675)
	{
		return true;
	}
	else if(r >= 750 && r <= 830)
	{
		return true;
	}
	else
	{
		return false;
	}
}

RealImage
GaussianFilter1D(real gauss_sgm, real epsilon) {

  assert(epsilon < 1.0);

  int fil_size=(int)(sqrt(-2.0*gauss_sgm*gauss_sgm*log(epsilon)) + .5);

  fil_size = 2*(fil_size-1)+1;
  //cout << "Gaussian filter size= " << fil_size << endl;

  RealImage result(1, 1, fil_size);

  int i;
  int center = (fil_size-1)/2;
  real inv_sgm = 1.0/(2.0*gauss_sgm*gauss_sgm);

  real val;
  real sum=0.0;
  for(i=0; i<fil_size; ++i) {
    val = (i-center)*(i-center)*inv_sgm;
    val = exp(-val);
    result.SetPixel(i, val);
    sum+=val;
  }
  result *=(1.0/sum);
  return result;
}

void
DiffusionFit(RealImage& f, const RealImage& g, real rate, real difspeed)
{
	int i, j, k;
	for(k=0; k<f.NumBands(); ++k)
	{
		for(i=0; i<f.NumRows(); ++i)
		{
			for(j=0; j<f.NumCols(); ++j)
			{
				real cf = f.GetPixel(k,i,j);
				real cg = g.GetPixel(k,i,j);
				real cn = f.GetPixelDefault(k, i-1, j, cf);
				real cw = f.GetPixelDefault(k, i, j-1, cf);
				real ce = f.GetPixelDefault(k, i, j+1, cf);
				real cs = f.GetPixelDefault(k, i+1, j, cf);

				real df = cg - cf;
				real dd = 0.25 * difspeed * ((cn-cf)+(cw-cf)+(ce-cf)+(cs-cf));
				real nf = cf + rate*df + dd;
				/*if(i==2 && j==2)
				{
					cout << cf << " " << df << " " << dd << " " << nf << endl;
				}*/
				f.SetPixel(k, i, j, nf);
			}
		}
	}
	return;
}
