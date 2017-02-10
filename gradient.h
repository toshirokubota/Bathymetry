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
#ifndef MATHS_OPTIMIZATION_GRADIENT_H
#define MATHS_OPTIMIZATION_GRADIENT_H

#include <vector>
#include <cmath>
using namespace std;

namespace Maths
{

	namespace Optimization
	{



		std::vector<double>
			gradient(double (*f)(const std::vector<double> &),
			std::vector<double> x, double xk = 1, double eps = 1E-10, int maxit = 1000) {

				int w = x.size();
				std::vector<double> d(w, 1 / sqrt((double)w)), y(w), z(w);

				double dd = 1;

				/////!!!!!for (int i = 0; i < w; y[2] = f(x), i++) {
				for (int i = 0; i < w; i++) {
					y[i] = f(x);

					dd = 0;
					for (int j = 0; j < w; dd += d[j] * d[j], j++);
					dd = sqrt(dd);

					z = x;
					for (int j = 0; j < w; x[j] += xk * d[j] / dd, j++);
				}

				int n = 0;
				do {
					if (fabs(y[1] - y[0]) >= 1E-15)
						if ((y[2] - y[1]) / (y[1] - y[0]) > 0)
							xk *= 1.2;

					if (y[2] < y[1]) xk /= 2;

					bool flag = true;
					if (y[2] <= y[1])
						x = z, flag = false;

					if (flag) y[0] = y[1], y[1] = y[2];

					y[2] = f(x);

					for (int i = 0; i < w; i++) {

						double a = x[i], b = d[i] * xk / (2 * dd);

						x[i] += b;
						double yy = f(x);

						if (b == 0) b = 1E-12;

						d[i] = (yy - y[2]) / b;
						if (d[i] == 0) d[i] = 1E-5;

						x[i] = a, yy = y[2];

					}

					dd = 0.0;
					for (int i = 0; i < w; dd += d[i] * d[i], i++);
					dd = sqrt(dd);

					if (dd == 0) return x;

					z = x;
					for (int i = 0; i < w; x[i] += xk * d[i] / dd, i++);

					y[2] = f(x);

					n++;

				} while (n < maxit && fabs(y[2] - y[1]) >= eps);

				return x;
			}

	};

};

#endif