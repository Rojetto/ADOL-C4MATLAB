#include "math.h"
#include "mex.h"
#include "adolc\adolc.h"
#include "madhelpers.h"
#include "invMat.h"


//Berechnung der Determinanten

double detrm(double** a, int k)
{
	double s = 1, det = 0;
	double** b = myalloc2(k, k);
	int i, j, m, n, c, f;

	if (k == 1)
	{
		return (a[0][0]);
	}
	else
	{
		det = 0;

		for (c = 0; c < k; c++)
		{
			m = 0;
			n = 0;

			for (i = 0; i < k; i++)
			{
				for (j = 0; j < k; j++)
				{
					b[i][j] = 0;

					if (i != 0 && j != c)
					{
						b[m][n] = a[i][j];

						if (n < k - 2)
							n++;
						else
						{
							n = 0;
							m++;
						}
					}
				}
			}
			f = k - 1;
			det = det + s*(a[0][c]*detrm(b, f));
			s = -1.0*s;
		}
	}

	myfree2(b);

	return (det);
}