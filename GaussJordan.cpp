#include "StdAfx.h"
#include "GaussJordan.h"

#define NR_END 1
#define FREE_ARG char*
const int GAUSS_JORDAN_SINGULAR = -1;

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

typedef double* pdouble;

void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;

	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) nrerror("allocation failure in ivector()");
	return v-nl+NR_END;
}

void free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with ivector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

bool gaussj(double **a, int n, double **b, int m){
	int *indxc,*indxr,*ipiv;
	int i,icol,irow,j,k,l,ll;
	double big,dum,pivinv,temp;

	indxc=ivector(1,n);
	indxr=ivector(1,n);
	ipiv=ivector(1,n);
	icol = -1;
	bool bSuccess = true;
	for (j=1;j<=n;j++) ipiv[j]=0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if (ipiv[j] != 1)
				for (k=1;k<=n;k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j][k]) >= big) {
							big=fabs(a[j][k]);
							irow=j;
							icol=k;
						}
					} else if (ipiv[k] > 1) {
						fprintf(stderr, "gaussj: Singular Matrix-1\n");
						bSuccess = false;
						goto CLEAN_UP;
					}
				}
				if (icol<0) {
					printf("Singular matrix???\n");
					int i2;
					/*for(i2=0; i2<n; ++i2) {
						for (int j2=0; j2<n; ++j2) {
							printf("%3.3f\t", a[i2][j2]);
						}
						printf("\n");
					}*/
					bSuccess = false;
					goto CLEAN_UP;
					//throw GAUSS_JORDAN_SINGULAR;
				}

				++(ipiv[icol]);
				if (irow != icol) {
					for (l=1;l<=n;l++) SWAP(a[irow][l],a[icol][l])
						for (l=1;l<=m;l++) SWAP(b[irow][l],b[icol][l])
				}
				indxr[i]=irow;
				indxc[i]=icol;
				if (a[icol][icol] == 0.0) {
					fprintf(stderr, "gaussj: Singular Matrix-2\n");
					bSuccess = false;
					goto CLEAN_UP;
				}
				pivinv=1.0/a[icol][icol];
				a[icol][icol]=1.0;
				for (l=1;l<=n;l++) a[icol][l] *= pivinv;
				for (l=1;l<=m;l++) b[icol][l] *= pivinv;
				for (ll=1;ll<=n;ll++)
					if (ll != icol) {
						dum=a[ll][icol];
						a[ll][icol]=0.0;
						for (l=1;l<=n;l++) a[ll][l] -= a[icol][l]*dum;
						for (l=1;l<=m;l++) b[ll][l] -= b[icol][l]*dum;
					}
	}
	for (l=n;l>=1;l--) {
		if (indxr[l] != indxc[l])
			for (k=1;k<=n;k++)
				SWAP(a[k][indxr[l]],a[k][indxc[l]]);
	}

CLEAN_UP:
	free_ivector(ipiv,1,n);
	free_ivector(indxr,1,n);
	free_ivector(indxc,1,n);

	return bSuccess;
}

bool
GaussJordan(const vector<real>& A, vector<real>& v, int n) {
	//GaussJordan(real* A, real* v, int n) {
	//int n=v.size();
	int m=1;
	int i,j;
	double** a;
	double** b;
	a=(pdouble*) malloc((n+1)*sizeof(pdouble));
	for(i=0; i<=n; ++i) 
		a[i]=(double*) malloc((n+1)*sizeof(double));
	b=(pdouble*) malloc((n+1)*sizeof(pdouble));
	for(i=0; i<=n; ++i) 
		b[i]=(double*) malloc((m+1)*sizeof(double));

	for(i=1; i<=n; ++i)
		for(j=1; j<=n; ++j)
			a[i][j]=A[(i-1)*n+(j-1)];
	for(i=1; i<=n; ++i)
		b[i][1]=v[i-1];

	bool bRet = gaussj(a,n,b,m);

	for(i=0; i<n; ++i)
		v[i]=b[i+1][1];

	for(i=0; i<=n; ++i) 
		free(a[i]);
	free(a);
	for(i=0; i<=n; ++i) 
		free(b[i]);
	free(b);

	return bRet;
}


