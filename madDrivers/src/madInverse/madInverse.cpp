// madInverse.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
extern "C" {
	#include "matrixlib.h"
}



// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_X			 0
#define MEXAD_IN_n			 1


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_I          0


// wird nach dem 1.Aufruf auf true gesetzt
static bool    MexInitialized = false;   

/* Für Matlab, damit Datei persistent wird - einmalige Zuordnung des File-Descriptors, um
 * Polling auf das Tape zu umgehen 
 */
static mxArray *persistent_array_ptr = NULL;


// Freigabe des Zugriffs auf das Tape und Rücksetzen der Initialisierung
// Muss hier so definiert werden, da mexAtExit einen Aufruf mit
// void parameterliste erwartet!
void cleanup(void) 
{
   mexPrintf("%s unloaded\n", __FILE__);
   mxDestroyArray(persistent_array_ptr);
   MexInitialized = false;
}


/* **************************************************************************
 * *****	Übergabeteil / Gateway-Routine								*****
 * *****	==============================								*****
 * *****																*****
 * *****	Programmeinsprungpunkt										*****
 * *****																*****
 * *****	Aufruf in MATLAB: I = madInverse(X,n)					    	*****
 * *****																*****
 * **************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* ptrOutput;					// Zeiger auf die Rückgabematrix
	double* ptrInput;					// Zeiger auf die Matrix zur Inversion
	int	n;


           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 2, 2);
	madCheckNumOutputs(nlhs, 0, 1);

	if (!CheckIfScalar(prhs, MEXAD_IN_n, "n")) return;
    n = (int)mxGetScalar(prhs[MEXAD_IN_n]);
	
	if (!madCheckDim2(prhs, MEXAD_IN_X, n, n, "X")) return;
	ptrInput = mxGetPr(prhs[MEXAD_IN_X]);
	
	// Rückgabe
	plhs[MEXAD_OUT_I] = mxCreateDoubleMatrix(n, n, mxREAL);
	ptrOutput = mxGetPr(plhs[MEXAD_OUT_I]);

	double** pX = myalloc2(n, n);

	madVector2Matrix(ptrInput, pX, n, n);

	double** pI = myalloc2(n, n);

	matError err;
	matMatrix *pIi = matMatrixCreateZero(n, n, &err);

	// Einlesen der Matrix 
	for (int i = 0; i < n; i++)				
	{
		for (int j = 0; j < n; j++)
		{
			matMatrixValSet(pIi, i, j, pX[i][j]);
		}
	}

	// Invertierung der Matrix
	pIi = matMatrixCreateInv(pIi, &err);

	// Speichern der invertierten Matrix in pI
	for (int i = 0; i < n; i++)				
	{
		for (int j = 0; j < n; j++)
		{
			pI[i][j]=matMatrixValGet(pIi, i, j, &err);
		}
	}

	matMatrixDelete(&pIi);
	
	madMatrix2Vector(pI, ptrOutput, n, n);
    
	myfree2(pX);
	myfree2(pI);

	return;
}