// CodeCogs Commercial License Agreement
// Copyright (C) 2004-2005 CodeCogs, Zyba Ltd, Broadwood, Holford, TA5 1DU, England.
//
// This software is licensed to <?php echo $_SESSION['firstname'].' '.$_SESSION['surname'] ?> 
// for commercial usage using version 1.2 of the CodeCogs Commercial Licence. You must 
// read this License (available at www.codecogs.com) before using this software.
//
// If you distribute this file it is YOUR responsibility to ensure that all 
// recipients have a valid number of commercial licenses. You must retain a
// copy of this licence in all copies you make. 
//
// This program is distributed WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the CodeCogs Commercial Licence for more details.
//---------------------------------------------------------------------------------
#ifndef MATHS_OPTIMIZATION_NELDER_H
#define MATHS_OPTIMIZATION_NELDER_H

#include <string.h>
#define ABS(x) ((x) < 0 ? -(x) : (x))

namespace Maths
{

namespace Optimization
{



void nelder(double (*f)(double *, const void*), 
			int dim, 
			double **p,
            double eps = 1E-10, 
			int maxit = 1000,
			const void* pParam = 0) {

    double *a = new double [dim], *b = new double [dim],
           *c = new double [dim], *y = new double [dim + 1];

    for (int i = 0; i <= dim; y[i] = f(p[i], pParam), i++);

    int t = 0;
    do {

        int k = 1, r = 0, q = 0;
        if (y[1] > y[2]) k = 0, r = 1;

        for (int i = 0; i <= dim; i++) {
            if (y[i] < y[q]) q = i;
            if (y[i] > y[k])
                r = k, k = i;
            else
                if (y[i] > y[r])
                    if (i != k) r = i;
        }

        double x = 2 * ABS(y[k] - y[q]) / (ABS(y[k]) + ABS(y[q]));

		//cout << "error = " << x << endl;
        if (x < eps || t == maxit) {
            delete [] a, delete [] b, delete [] c;
			//cout << "done" << endl;
            return;
        }
        t++;

        memset(c, 0, dim * sizeof(double));
        for (int i = 0; i <= dim; i++)
        {
            if (i != k)
                for (int j = 0; j < dim; j++)
                    c[j] += p[i][j];
        }

        for (int j = 0; j < dim; j++)
            c[j] /= dim, a[j]  = 2 * c[j] - p[k][j];
        double fa = f(a, pParam);

        if (fa <= y[q]) 
        {
            for (int j = 0; j < dim; b[j] = 2 * a[j] - c[j], j++);
            double fb = f(b, pParam);

            memcpy(p[k], fb < y[q] ? b : a, dim * sizeof(double));
            y[k] = fb < y[q] ? fb : fa;

        }
        else
            if (fa >= y[r]) {

                if (fa < y[k]) {
                    memcpy(p[k], a, dim * sizeof(double));
                    y[k] = fa;
                }

                for (int j = 0; j < dim; j++) b[j] = (p[k][j] + c[j]) / 2;
                double fb = f(b, pParam);

                if (fb < y[k]) {
                    memcpy(p[k], b, dim * sizeof(double));
                    y[k] = fb;
                }
                else
                    for (int i = 0; i <= dim; i++)
                        if (i != q) {
                            for (int j = 0; j < dim; j++)
                                p[i][j] = a[j] = (p[i][j] + p[q][j]) / 2;
                            y[i] = f(a, pParam);
                        }

            }
            else {
                memcpy(p[k], a, dim * sizeof(double));
                y[k] = fa;
            }

    } while (true);

}

};

};

#undef ABS

#endif