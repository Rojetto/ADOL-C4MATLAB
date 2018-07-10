/**
 * (c) 2014-2018 
 * Jan Winkler
 * Institute of Control Theory
 * Technische Universität Dresden
 * Jan.Winkler@tu-dresden.de
 */

#ifndef MATRIX_LIB_H
#define MATRIX_LIB_H

#include <stddef.h>
#include <stdint.h>


typedef uint16_t matIndex;
typedef double   matDataType;

typedef struct
{
	matDataType* Z;	/* The array holding the matrix entries */
	matIndex N;		/* Number of rows */
	matIndex M;		/* Number of columnss */
} matMatrix;


typedef enum
{
	matOk,					/* Operation succesfull */
	matSing,				/* Matrix is singular */
	matDimMismatchAdd,		/* Addition not possible due to dimension mismatch */
	matDimMismatchMul,		/* Multiplication not possible due to dimension mismatch */
	matRange,				/* Passed row and/or column index exeeds matrix dimension */
	matNotQuadr,			/* Operation not possible because the matrix is not quadratic */
	matMemError,			/* Memory allocation failed */
	matNullPtr				/* Operation not possible because a Null pointer has passed */
} matError;



/* Creates a zero matrix with n rows and m columns */
matMatrix* matMatrixCreateZero( matIndex n, matIndex m, matError* const err);

/* Creates an matrix with n rows and m columns the diagonal elements of which are 1, the rest 0*/
matMatrix* matMatrixCreateEye( matIndex n, matIndex m, matError* const err);

/* Creates a matrix with n rows and m columns the elements of which are uniformly distributed
   random values between min and max */
matMatrix* matMatrixCreateRand( matIndex n, matIndex m, matDataType min, matDataType max, 
     							matIndex seed, matError* const err);


/* Deletes the memory allocated for the matrix the pointer A* points to */
matError   matMatrixDelete( matMatrix** A );

/* Creates a new matrix which is equal to the matrix A points to */
matMatrix* matMatrixCpy( const matMatrix* const A, matError* const err );

/* Sets the element in the n.th row and the m.th column of the matrix A points to to the value x */
matError   matMatrixValSet( matMatrix* const A, matIndex n, matIndex m, matDataType x );

/* Returns the element in the n.th row and the m.th column of the matrix A points to */
matDataType     matMatrixValGet( matMatrix* const A, matIndex n, matIndex m, matError* const err );



/* Adds two matrices A and B and returns a pointer to a new matrix holding the result */
matMatrix* matMatrixCreateAdd( const matMatrix* const A, const matMatrix* const B, matError* const err );

/* Substracts two matrices A and B and returns a pointer to a new matrix holding the result */
matMatrix* matMatrixCreateSub( const matMatrix* const A, const matMatrix* const B, matError* const err );

/* Multiplies two matrices A and B and returns a pointer to a new matrix holding the result */
matMatrix* matMatrixCreateMul( const matMatrix* const A, const matMatrix* const B, matError* const err );

/* Multiplies three matrices A, B, and C and returns a pointer to a new matrix holding the result */
//matMatrix* matMatrixMul3( const matMatrix* const A, const matMatrix* const B, const matMatrix* const C );

/* Multiplies four matrices A, B, C, and D and returns a pointer to a new matrix holding the result */
//matMatrix* matMatrixMul4( const matMatrix* const A, const matMatrix* const B,
//						  const matMatrix* const C, const matMatrix* const D);


/* Calculates the inverse of a quadratic matrix A points to and returns a pointer to a new matrix holding the result */
matMatrix* matMatrixCreateInv( const matMatrix* const A, matError* const err );

/* Calculates the transpose of a quadratic matrix A points to and returns a pointer to a new matrix holding the result */
matMatrix* matMatrixCreateTra( const matMatrix* const A, matError* const err );

/* Negates the matrix A points to and returns a pointer to a new matrix holding the result */
matMatrix* matMatrixCreateNeg( const matMatrix* const A, matError* const err  );



/* Inverts the matrix A points to */
//matError   matMatrixInv( matMatrix* const A );

/* Transposes the matrix A points to */
matError   matMatrixTra( matMatrix* const A );

/* Negates the matrix A points to */
matError   matMatrixNeg( matMatrix* const A );



/* Adds a scalar x to all elements of the matrix A points to */
//matError   matMatrixScalarAdd( matMatrix* const A, matDataType x );

/* Multiplies all elements of the matrix A points to with the scalar x */
//matError   matMatrixScalarMul( matMatrix* const A, matDataType x );


/* Checks if the matrix A points to is an empty (0 X 0) matrix */
unsigned char matMatrixIsEmpty( const matMatrix* const A );

/* Checks if the matrix A points to is a zero-matrix */
unsigned char matMatrixIsZero( const matMatrix* const A, matError* const err );

/* Checks if the matrix A points to is an identity-matrix */
unsigned char matMatrixIsEye( const matMatrix* const A, matError* const err );

/* Checks if the matrix A points to is an diagonal-matrix */
unsigned char matMatrixIsDiag( const matMatrix* const A, matError* const err );


/* Calculates the determinant of a quadratic matrix A */
//matDataType matMatrixDet( const matMatrix* const A );


matError matMatrixPrint( const matMatrix* const A );




#endif