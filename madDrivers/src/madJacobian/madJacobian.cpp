// madJacobian.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"

using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_X			 1


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_J          0


// wird nach dem 1.Aufruf auf true gesetzt
static bool    MexInitialized = false;   

// signalisiert, das die Tapes geschrieben wurden bzw. vorhanden sind
static bool    TapesWritten   = false;   

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
 * *****	Aufruf in MATLAB: J = madJacobian(TapeID, X)				*****
 * *****																*****
 * **************************************************************************
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	// Variablendefinitionen für die Verwendung 
	double* ptrOutput;					// Zeiger auf die Rückgabematrix
	double* pX;							// Zeiger auf die Matrix für Taylor-Koeff.
	// der unabh. Variablen

	MexADCTagType TapeID;				// Tape-Kennzeichner
	int m;								// Anzahl der abh. Variablen
	int n;								// Anzahl der unabh. Variablen

	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
	size_t TapeInfo[STAT_SIZE];


	// Initialisierung der MEX-Funktion
	if (!MexInitialized)
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);


	// Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 2, 2);
	madCheckNumOutputs(nlhs, 0, 1);


	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId")) return;
	TapeID = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]);

	//	Anzahl der "n" unabhängigen und "m" abhängigen Variablen des Tapes
	tapestats(TapeID, TapeInfo);
	n = TapeInfo[0];
	m = TapeInfo[1];

	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	// Rückgabe
	plhs[MEXAD_OUT_J] = mxCreateDoubleMatrix(m, n, mxREAL);
	ptrOutput = mxGetPr(plhs[MEXAD_OUT_J]);



	//	Aufruf der Berechnungsprozedur
	double** pJ = myalloc2(m, n);

	jacobian(TapeID, m, n, pX, pJ);

	madMatrix2Vector(pJ, ptrOutput, m, n);

	myfree2(pJ);

	return;
}


