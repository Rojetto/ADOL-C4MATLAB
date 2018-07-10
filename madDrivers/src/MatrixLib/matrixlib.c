/**
 * (c) 2014-2018 
 * Jan Winkler
 * Institute of Control Theory
 * Technische Universität Dresden
 * Jan.Winkler@tu-dresden.de
 */

#include "matrixlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//#define MAT_STAMP 0x19D8B15
//#define MAT_IS_VALID(X) ( (X != NULL) && ( (X)->Valid == MAT_STAMP ) )
#define MAT_IS_VALID(X) (X != NULL)
#define MAT_SWAP_INDEX(X, Y) {matIndex    Temp = X; X = Y; Y = Temp;}
#define MAT_SWAP_DATA(X, Y)  {matDataType Temp = X; X = Y; Y = Temp;}

matMatrix* matMatrixCreateZero( matIndex n, matIndex m, matError* const err )
{
	matMatrix* A;

	A = (matMatrix*)malloc(sizeof(matMatrix));
	if (A == NULL)
	{
		*err = matMemError;
		return NULL;
	}

	A->Z = (matDataType*)calloc(n*m, sizeof(matDataType));
	if (A->Z == NULL)
	{
		free(A);
		*err = matMemError;
		return NULL;
	}

	A->N     = n;
	A->M     = m;

	*err = matOk;
	return A;
}



matMatrix* matMatrixCreateEye( matIndex n, matIndex m, matError* const err)
{
	matIndex i, iMax;
	matMatrix* A = matMatrixCreateZero( n, m, err );
	if (A == NULL) return NULL;

	iMax = A->M > A->N ? A->N : A->M;
	for (i = 0; i < iMax; i++)
	{
		A->Z[A->M*i + i] = (matDataType)1;
	};

	
	*err = matOk;
	return A;
}


matMatrix* matMatrixCreateRand( matIndex n, matIndex m, matDataType min, matDataType max, 
							    matIndex seed, matError* const err)
{
	matIndex i, j;
	matDataType delta;

	matMatrix* A = matMatrixCreateZero(n, m, err);
	if (A == NULL) return NULL;

	delta = max - min;
	srand(seed);
	for (i = 0; i < A->N; i++)
	{
		for (j = 0; j < A->M; j++)
		{
			A->Z[A->M*i+j] = min + delta*rand()/RAND_MAX;
		}
	}
	*err = matOk;
	return A;
}

matMatrix* matMatrixCpy( const matMatrix* const A, matError* const err )
{
	matIndex i, NumEle;
	matMatrix* B;

	if (!MAT_IS_VALID(A))
	{
		*err = matNullPtr;
		return NULL;
	}

	B = matMatrixCreateZero(A->N, A->M, err);
	if (B == NULL) return NULL;
	
	NumEle = A->N*A->M;
	for (i = 0; i < NumEle; i++)
	{
		B->Z[i] = A->Z[i];
	}

	*err = matOk;
	return B;
}



matError matMatrixDelete( matMatrix** A )
{
	if (!MAT_IS_VALID(*A))
	{
		return matNullPtr;
	}
	else
	{
		(*A)->M     = 0;
		(*A)->N     = 0;
		free((*A)->Z);
		free(*A);
		*A = NULL;
		return matOk;
	};
}



matError matMatrixValSet( matMatrix* const A, matIndex n, matIndex m, matDataType x )
{
	if (!MAT_IS_VALID(A))			  return matNullPtr;
	if ( (n > A->N) || (m > A->M) )   return matRange;
	A->Z[A->M*n+m] = x;
	return matOk;
}



matDataType     matMatrixValGet( matMatrix* const A, matIndex n, matIndex m, matError* const err )
{
	if (!MAT_IS_VALID(A))			  
	{
		*err = matNullPtr;
		return 0.0;
	}
	if ( (n > A->N) || (m > A->M) )   
	{
		*err = matRange;
		return 0.0;
	}

	return A->Z[A->M*n+m];
}


matError matMatrixPrint( const matMatrix* const A )
{
	matIndex i, j;

	if (!MAT_IS_VALID(A)) return matNullPtr;

	printf("\n");
	for (i=0; i < A->N; i++)
	{
		for (j=0; j < A->M; j++)
		{
			printf("%.4f\t", A->Z[A->M*i+j]);
		};
		printf("\n");
	};
	printf("\n");

	return matOk;
}



unsigned char matMatrixIsEmpty( const matMatrix* const A )
{
	if (!MAT_IS_VALID(A)) return 1;

	return A->Z == NULL ? 1 : 0;
}



unsigned char matMatrixIsZero( const matMatrix* const A, matError* const err )
{
	matIndex NumEle;
	matIndex i = 0;

	if (!MAT_IS_VALID(A)) 
	{
		*err = matNullPtr;
		return 0;
	};

	NumEle = A->M*A->N;
	while ( i < NumEle )
	{
		if (A->Z[i] != 0) return 0;
		i++;
	}
	return 1;
}

/* Checks if the matrix A points to is an eye-matrix */
unsigned char matMatrixIsEye( const matMatrix* const A, matError* const err )
{
	matIndex i, j;

	if (!MAT_IS_VALID(A)) 
	{
		*err = matNullPtr;
		return 0;
	};

	for (i=0; i < A->N; i++)
	{
		for (j=0; j < A->M; j++)
		{
			if ( ((i != j) && (A->Z[A->M*i+j] != 0)) || ((i == j) && (A->Z[A->M*i+j] != 1)) ) return 0;
		}
	}

	return 1;
}



unsigned char matMatrixIsDiag( const matMatrix* const A, matError* const err )
{
	matIndex i, j;

	if (!MAT_IS_VALID(A)) 
	{
		*err = matNullPtr;
		return 0;
	};

	for (i=0; i < A->N; i++)
	{
		for (j=0; j < A->M; j++)
		{
			if ( (i != j) && (A->Z[A->M*i+j] != 0) ) return 0;
		}
	}

	return 1;
}



matMatrix* matMatrixCreateAdd( const matMatrix* const A, const matMatrix* const B, matError* const err )
{
	matMatrix* C;
	matIndex i, NumEle;

	if ( (!MAT_IS_VALID(A)) || (!MAT_IS_VALID(B)) )       
	{
		*err = matNullPtr;
		return NULL;
	};

	if ( (A->N != B->N) || (A->M != B->M) )
	{
		*err = matDimMismatchAdd;
		return NULL;
	};

	C = matMatrixCreateZero(A->N, A->M, err);
	if (C == NULL) return NULL;

	NumEle = C->N*C->M;
	for (i=0; i < NumEle; i++)
		C->Z[i] = A->Z[i] + B->Z[i];

	*err = matOk;
	return C;
}


matMatrix* matMatrixCreateSub( const matMatrix* const A, const matMatrix* const B, matError* const err )
{
	matMatrix* C;
	matIndex i, NumEle;

	if ( (!MAT_IS_VALID(A)) || (!MAT_IS_VALID(B)) )       
	{
		*err = matNullPtr;
		return NULL;
	};

	if ( (A->N != B->N) || (A->M != B->M) )
	{
		*err = matDimMismatchAdd;
		return NULL;
	};

	C = matMatrixCreateZero(A->N, A->M, err);
	if (C == NULL) return NULL;

	NumEle = C->N*C->M;
	for (i=0; i < NumEle; i++)
		C->Z[i] = A->Z[i] - B->Z[i];

	*err = matOk;
	return C;
}


matMatrix* matMatrixCreateMul( const matMatrix* const A, const matMatrix* const B, matError* const err )
{
	matMatrix* C;
	matIndex i, j, k;

	if ( (!MAT_IS_VALID(A)) || (!MAT_IS_VALID(A)) )       
	{
		*err = matNullPtr;
		return NULL;
	};

	if (A->M != B->N)
	{
		*err = matDimMismatchMul;
		return NULL;
	};

	C = matMatrixCreateZero(A->N, B->M, err);
	if (C == NULL) return NULL;

	for (i=0; i < C->N; i++)
	{
		for (j=0; j < C->M; j++)
		{
			for (k=0; k < C->N; k++)
			{
				C->Z[C->M*i+j] += A->Z[A->M*i+k]*B->Z[B->M*k+j];
			};
		};
	};

	*err = matOk;
	return C;
}

matMatrix* matMatrixCreateNeg( const matMatrix* const A, matError* const err )
{
	matMatrix* C;
	matIndex i, NumEle;

	if (!MAT_IS_VALID(A))
	{
		*err = matNullPtr;
		return NULL;
	};

	C = matMatrixCreateZero(A->N, A->M, err);
	NumEle = A->N*A->M;
	for (i = 0; i < NumEle; i++)
		C->Z[i] = -A->Z[i];

	*err = matOk;
	return C;

}

matMatrix* matMatrixCreateTra( const matMatrix* const A, matError* const err )
{
	matMatrix* C;
	matIndex i, j;

	if (!MAT_IS_VALID(A))
	{
		*err = matNullPtr;
		return NULL;
	};

	C = matMatrixCreateZero(A->M, A->N, err);
	for (i=0; i < A->N; i++)
	{
		for (j=0; j < A->M; j++)
		{
			C->Z[C->M*j+i] = A->Z[A->M*i+j];
		}
	}
	
	*err = matOk;
	return C;
}


matMatrix* matMatrixCreateInv( const matMatrix* const A, matError* const err )
{
	matMatrix *C, *D;
	matIndex* P;
	matIndex i, j, k, l;
	matDataType piv, fac;

	if (!MAT_IS_VALID(A))
	{
		*err = matNullPtr;
		return NULL;
	};

	if ( A->N != A->M )
	{
		*err = matNotQuadr;
		return NULL;
	};
	
	C = matMatrixCreateZero(A->N, A->N, err);
	if (C == NULL) return NULL;

	D = matMatrixCreateZero(A->N, A->N, err);
	if (D == NULL) return NULL;

	P = (matIndex*)calloc(A->N, sizeof(matIndex));
	if (P == NULL) 
	{
		*err = matMemError;
		return NULL;
	};

	for (i = 0; i < A->N; i++)
	{
		matMatrixValSet(C, i, i, 1.0);
		P[i] = i;
	};

	/* Loop over the columns of A */
	for (i = 0; i < A->M; i++)
	{
		/* Find Pivot-Element: Search on which column the largest */
		/* potential diagonal element can be found */
		l = i;
		for (j = i+1; j < A->M; j++)
		{
			if ( fabs(A->Z[A->M*i+P[j]]) > fabs(A->Z[A->M*i+P[l]]) ) l = j;
		};
		if ( i != l )
		{
			MAT_SWAP_INDEX(P[i], P[l]);
		};

		/* Normalize i.th row such that a_{ii} is 1, i.e. */
		/* devide each element of the row by a_{ii} */
		piv = A->Z[A->M*i+P[i]];
		if (piv == 0) /* Matrix is singular! */
		{
			matMatrixDelete(&C);
			matMatrixDelete(&D);
			free(P);
			*err = matSing;
			return NULL;
		};
		for (j = 0; j < A->N; j++)
		{
			C->Z[C->M*i+j]    = C->Z[C->M*i+j]/piv;
			A->Z[C->M*i+P[j]] = A->Z[C->M*i+P[j]]/piv;
		}

		/* Now eliminate non diagonal elements of the i.th column */
		/* i.e. only the i.th element in the i.th column (which is 1) remains */
		for (k = 0; k < A->N; k++)
		{
			if (k != i)
			{
				fac = A->Z[A->M*k+P[i]];
				for (j = 0; j < A->M; j++)
				{
					C->Z[C->M*k+j]    = C->Z[C->M*k+j]    - fac*C->Z[C->M*i+j];
					A->Z[A->M*k+P[j]] = A->Z[A->M*k+P[j]] - fac*A->Z[A->M*i+P[j]];
				}
			}
		}

	}

	/* Re-arrange C to compensate pivot-operations and return matrix*/
	for (i = 0; i < C->N; i++)
	{
		for (j = 0; j < C->M; j++)
		{
			D->Z[D->M*P[i]+j] = C->Z[C->M*i+j];
		}
	}

	matMatrixDelete(&C);
	free(P);
	*err = matOk;
	return D;
}


matError matMatrixTra( matMatrix* const A )
{
	matIndex i, j, NumEle;
	matDataType* ZNew;

	if (!MAT_IS_VALID(A)) return matNullPtr;

	/* Certainly not the most elegant way, but as a first step... */
	NumEle = A->N*A->M;
	ZNew = (matDataType*)calloc(NumEle, sizeof(matDataType));

	for (i=0; i < A->N; i++)
	{
		for (j=0; j < A->M; j++)
		{
			ZNew[A->N*j+i] = A->Z[A->M*i+j];
		}
	}

	free(A->Z);
	A->Z = ZNew;
	i    = A->M;
	A->M = A->N;
	A->N = i;

	return matOk;
}


matError matMatrixNeg( matMatrix* const A )
{
	matIndex i, NumEle;
	if (!MAT_IS_VALID(A)) return matNullPtr;
	NumEle = A->N*A->M;

	for (i=0; i < NumEle; i++)
	{
		A->Z[i] = -A->Z[i];
	}
	
	return matOk;
}

