// madInverse2.cpp


// Ben�tigte Header und Namensr�ume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"




// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_X			 0
#define MEXAD_IN_n			 1


// Position und Bedeutung der R�ckgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_I          0


// wird nach dem 1.Aufruf auf true gesetzt
static bool    MexInitialized = false;   

/* F�r Matlab, damit Datei persistent wird - einmalige Zuordnung des File-Descriptors, um
 * Polling auf das Tape zu umgehen 
 */
static mxArray *persistent_array_ptr = NULL;


// Freigabe des Zugriffs auf das Tape und R�cksetzen der Initialisierung
// Muss hier so definiert werden, da mexAtExit einen Aufruf mit
// void parameterliste erwartet!
void cleanup(void) 
{
   mexPrintf("%s unloaded\n", __FILE__);
   mxDestroyArray(persistent_array_ptr);
   MexInitialized = false;
}


/* **************************************************************************
 * *****	�bergabeteil / Gateway-Routine								*****
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
	// Variablendefinitionen f�r die Verwendung 
	double* ptrInput;					// Zeiger auf die Matrix zur Inversion
	int	n;
          

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Pr�fen der Anzahl der Eingabe- und R�ckgabeargumente
	madCheckNumInputs(nrhs, 2, 2);
	madCheckNumOutputs(nlhs, 0, 1);

	if (!CheckIfScalar(prhs, MEXAD_IN_n, "n")) return;
    n = (int)mxGetScalar(prhs[MEXAD_IN_n]);
	
	if (!madCheckDim2(prhs, MEXAD_IN_X, n, n, "X")) return;
	ptrInput = mxGetPr(prhs[MEXAD_IN_X]);

	mxArray *prhs_temp[1];
	prhs_temp[0] = mxCreateDoubleMatrix(n, n, mxREAL);
	double *lsv;
	lsv = mxGetPr(prhs_temp[0]);
	for (int i = 0; i < n*n; i++)				
	{
		lsv[i] = ptrInput[i];
	}

	mexCallMATLAB(1, plhs, 1, prhs_temp, "inv");
	
	mxDestroyArray(prhs_temp[0]);

	return;
}