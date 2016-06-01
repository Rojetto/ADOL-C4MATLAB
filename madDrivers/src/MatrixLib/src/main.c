#include "matrixlib.h"

int main(void)
{
	matMatrix *A = NULL;
	matMatrix *B = NULL; 
	matMatrix *C = NULL; 
	matMatrix *D = NULL; 
	matMatrix *E = NULL; 
	matMatrix *F = NULL; 
	matMatrix *G = NULL; 
	matMatrix *R = NULL;
	matMatrix *Z = NULL;
	matError err;

	A = matMatrixCreateZero(3, 3, &err);
	B = matMatrixCreateZero(3, 3, &err);
	C = matMatrixCreateZero(3, 4, &err);
	Z = matMatrixCreateZero(5, 5, &err);

	/*
	R = matMatrixCreateRand(10, 10, 1, 100, 12345, &err);
	matMatrixPrint(R);
	Z = matMatrixCreateInv(R, &err);
	matMatrixPrint(Z);
	matMatrixDelete(&Z);
	matMatrixDelete(&R);
	*/

	matMatrixValSet(A, 0, 0, 2);
	matMatrixValSet(A, 0, 1, 4);
	matMatrixValSet(A, 0, 2, 6);

	matMatrixValSet(A, 1, 0, 8);
	matMatrixValSet(A, 1, 1, 10);
	matMatrixValSet(A, 1, 2, 12);

	matMatrixValSet(A, 2, 0, 14);
	matMatrixValSet(A, 2, 1, 16);
	matMatrixValSet(A, 2, 2, 18);

	
	matMatrixValSet(B, 0, 0, 1);
	matMatrixValSet(B, 0, 1, 2);
	matMatrixValSet(B, 0, 2, 3);

	matMatrixValSet(B, 1, 0, 4);
	matMatrixValSet(B, 1, 1, 5);
	matMatrixValSet(B, 1, 2, 6);

	matMatrixValSet(B, 2, 0, 7);
	matMatrixValSet(B, 2, 1, 8);
	matMatrixValSet(B, 2, 2, 9);



	matMatrixValSet(C, 0, 0, 2);
	matMatrixValSet(C, 0, 1, 4);
	matMatrixValSet(C, 0, 2, 6);
	matMatrixValSet(C, 0, 3, 8);

	matMatrixValSet(C, 1, 0, 10);
	matMatrixValSet(C, 1, 1, 12);
	matMatrixValSet(C, 1, 2, 14);
	matMatrixValSet(C, 1, 3, 16);

	matMatrixValSet(C, 2, 0, 18);
	matMatrixValSet(C, 2, 1, 20);
	matMatrixValSet(C, 2, 2, 22);
	matMatrixValSet(C, 2, 3, 24);


	matMatrixPrint(A);
	matMatrixPrint(B);

	D = matMatrixCreateAdd(A, B, &err);
	matMatrixPrint(D);

	E = matMatrixCreateMul(A, B, &err);
	matMatrixPrint(E);

	matMatrixPrint(C);
	F = matMatrixCreateTra(C, &err);
	matMatrixPrint(F);
	matMatrixTra(C);
	matMatrixPrint(C);



	matMatrixValSet(A, 0, 0, 2);
	matMatrixValSet(A, 0, 1, 6);
	matMatrixValSet(A, 0, 2, 4);

	matMatrixValSet(A, 1, 0, 2);
	matMatrixValSet(A, 1, 1, 8);
	matMatrixValSet(A, 1, 2, 5);

	matMatrixValSet(A, 2, 0, 1);
	matMatrixValSet(A, 2, 1, 4);
	matMatrixValSet(A, 2, 2, 7);

	/*
	matMatrixValSet(A, 0, 0, 1);
	matMatrixValSet(A, 0, 1, 1);
	matMatrixValSet(A, 0, 2, 1);

	matMatrixValSet(A, 1, 0, 0);
	matMatrixValSet(A, 1, 1, 1);
	matMatrixValSet(A, 1, 2, 1);

	matMatrixValSet(A, 2, 0, 0);
	matMatrixValSet(A, 2, 1, 0);
	matMatrixValSet(A, 2, 2, 1);
*/
/*
	matMatrixValSet(Z, 0, 0, 25);
	matMatrixValSet(Z, 0, 1, 59);
	matMatrixValSet(Z, 0, 2, 27);
	matMatrixValSet(Z, 0, 3, 94);
	matMatrixValSet(Z, 0, 4, 38);

	matMatrixValSet(Z, 1, 0, 68);
	matMatrixValSet(Z, 1, 1, 37);
	matMatrixValSet(Z, 1, 2, 43);
	matMatrixValSet(Z, 1, 3, 80);
	matMatrixValSet(Z, 1, 4, 56);

	matMatrixValSet(Z, 2, 0, 76);
	matMatrixValSet(Z, 2, 1, 77);
	matMatrixValSet(Z, 2, 2, 66);
	matMatrixValSet(Z, 2, 3, 54);
	matMatrixValSet(Z, 2, 4, 56);

	matMatrixValSet(Z, 3, 0, 68);
	matMatrixValSet(Z, 3, 1, 27);
	matMatrixValSet(Z, 3, 2, 80);
	matMatrixValSet(Z, 3, 3, 49);
	matMatrixValSet(Z, 3, 4, 84);

	matMatrixValSet(Z, 4, 0, 51);
	matMatrixValSet(Z, 4, 1, 72);
	matMatrixValSet(Z, 4, 2, 17);
	matMatrixValSet(Z, 4, 3, 50);
	matMatrixValSet(Z, 4, 4, 82);
	*/
	


	matMatrixValSet(Z, 0, 0, 1);
	matMatrixValSet(Z, 0, 1, 2);
	matMatrixValSet(Z, 0, 2, 3);
	matMatrixValSet(Z, 0, 3, 4);
	matMatrixValSet(Z, 0, 4, 5);

	matMatrixValSet(Z, 1, 0, 1);
	matMatrixValSet(Z, 1, 1, 2);
	matMatrixValSet(Z, 1, 2, 3);
	matMatrixValSet(Z, 1, 3, 4);
	matMatrixValSet(Z, 1, 4, 5);

	matMatrixValSet(Z, 2, 0, 0);
	matMatrixValSet(Z, 2, 1, 0);
	matMatrixValSet(Z, 2, 2, 3);
	matMatrixValSet(Z, 2, 3, 0);
	matMatrixValSet(Z, 2, 4, 0);

	matMatrixValSet(Z, 3, 0, 0);
	matMatrixValSet(Z, 3, 1, 0);
	matMatrixValSet(Z, 3, 2, 0);
	matMatrixValSet(Z, 3, 3, 4);
	matMatrixValSet(Z, 3, 4, 0);

	matMatrixValSet(Z, 4, 0, 0);
	matMatrixValSet(Z, 4, 1, 0);
	matMatrixValSet(Z, 4, 2, 0);
	matMatrixValSet(Z, 4, 3, 0);
	matMatrixValSet(Z, 4, 4, 5);

	matMatrixPrint(Z);
	//G = matMatrixCreateInv(Z, &err);
	matMatrixPrint(G);


	matMatrixDelete(&A);
	matMatrixDelete(&B);
	matMatrixDelete(&C);
	matMatrixDelete(&D);
	matMatrixDelete(&E);
	matMatrixDelete(&F);
	matMatrixDelete(&G);
	matMatrixDelete(&Z);
	
	return 0;
}