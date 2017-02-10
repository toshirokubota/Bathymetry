#include "rndm.h"
#include <cmath>
using namespace std;

double 
rndm(int jd)
{
   /*static int mdig = 16;*/	/* # digits used for integers */
/*
   M1= 2**(MDIG-2) + (2**(MDIG-2)-1),  M2 = 2**(MDIG/2)
*/

   static int m[17] =
   {30788, 23052, 2053, 19346, 10646, 19427, 23975,
   19049, 10949, 19693, 29746, 26748, 2796, 23890, 29168, 31924, 16499};

   static int i = 4, j = 16, m1 = 32767, m2 = 256;
   static double invm1 = .00003051850947599719;  /* 1.0 / m1 */

   int k, jseed, k0, k1, j0, j1;

   if (jd != 0)
   {
      jseed = (jd < 0) ? -jd : jd;
      jseed = (jseed < m1) ? jseed : m1;
      if (jseed % 2 == 0)
	 --jseed;
      k0 = 9069 % m2;
      k1 = 9069 / m2;
      j0 = jseed % m2;
      j1 = jseed / m2;
#if 0
      printf("jseed= %d, k0= %d, k1= %d, j0= %d, j1= %d\n", jseed, k0, k1, j0, j1);
#endif
      for (i = 0; i < 17; ++i)
      {
	 jseed = j0 * k0;
	 j1 = ((jseed / m2) + j0 * k1 + j1 * k0) % (m2 / 2);
	 j0 = jseed % m2;
	 m[i] = j0 + m2 * j1;
#if 0
	 printf("i= %d, m[i]= %d\n", i, m[i]);
#endif
      }
      i = 4;
      j = 16;
   }
   k = m[i] - m[j];
   if (k < 0)
      k += m1;
   m[j] = k;
   if (--i < 0)
      i = 16;
   if (--j < 0)
      j = 16;
   return (double) k *  invm1;
}

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

/**************************************************
 Park & Millers' random number generator from
 Numerical Recipes, p. 280
 Call with idum a negative integer to initialize;
 **************************************************/
double ran1(long *idum)
{
   int j;
   long k;
   static long iy=0;
   static long iv[NTAB];
   double temp;
   
   if (*idum <= 0 || !iy) {
      if (-(*idum)<1) *idum=1;
      else *idum = -(*idum);
      for (j=NTAB+7; j>=0; j--) {
         k = (*idum)/IQ;
         *idum = IA*(*idum-k*IQ)-IR*k;
         if (*idum<0) *idum += IM;
         if (j<NTAB) iv[j] = *idum;
      }
      iy = iv[0];
   }
   k = (*idum)/IQ;
   *idum = IA*(*idum-k*IQ)-IR*k;
   if (*idum<0) *idum += IM;
   j = iy/NDIV;
   iy = iv[j];
   iv[j] = *idum;
   if ((temp=AM*iy)>RNMX) return RNMX;
   else return temp;
}

double gasdev(long jd)
{
	static int iset=0;
	static double gset;
	double fac,r,v1,v2;
    //double ran1();

	if(jd != 0)
	{
		rndm(jd);
		jd = 0;
		iset = 0;
	}

	if  (iset == 0) {
		do {
			v1=2.0*rndm(jd)-1.0;
			v2=2.0*rndm(jd)-1.0;
			r=v1*v1+v2*v2;
		} while (r >= 1.0);
		fac=sqrt(-2.0*log(r)/r);
		gset=v1*fac;
		iset=1;
		return v2*fac;
		/*return .2132 * v2*fac;*/
	} else {
		iset=0;
		return gset;
		/*return .2132 * gset;*/
	}
}
