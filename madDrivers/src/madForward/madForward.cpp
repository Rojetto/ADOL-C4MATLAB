// madforward.cpp : Definiert die exportierten Funktion forward(tag,m,n,d,keep,X,Y) für die Einbindung als DLL.


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "adolc\lie\drivers.h"



// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_d		     1
#define MEXAD_IN_keep        2
#define MEXAD_IN_X			 3


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_Y          0


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
 * *****	Aufruf in MATLAB: Y = madforward(TapeID, d, keep, X)    	*****
 * *****																*****
 * **************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* ptrOutput;					// Zeiger auf die Rückgabematrix
	double* ptrInput;					// Zeiger auf die Matrix für Taylor-Koeff.
										// der unabh. Variablen
	
	MexADCTagType TapeID;				// Tape-Kennzeichner
    int m;								// Anzahl der abh. Variablen
	int n;								// Anzahl der unabh. Variablen
	int d;								// Ableitungsordnung (degree of derivation)
	int keep = 0;						// Flag für Rückwärtsmodus (default = 0)


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
    size_t TapeInfo[STAT_SIZE]; 
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 4, 4);
	madCheckNumOutputs(nlhs, 0, 1);
            

    // Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId")) return; 
	TapeID     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]); 
    
    //	Anzahl der "n" unabhängigen und "m" abhängigen Variablen des Tapes
    tapestats(TapeID, TapeInfo);
	n = TapeInfo[0];
    m = TapeInfo[1];  

    // Ableitungsordnung
	if (!CheckIfScalar(prhs, MEXAD_IN_d, "d")) return;
    d = (int)mxGetScalar(prhs[MEXAD_IN_d]);

	// Keep für reverse
	if (!CheckIfScalar(prhs, MEXAD_IN_keep, "keep")) return;
	keep = (int)mxGetScalar(prhs[MEXAD_IN_keep]);
	
	// X
	if (!madCheckDim2(prhs, MEXAD_IN_X, n, d+1, "X")) return;
	ptrInput = mxGetPr(prhs[MEXAD_IN_X]);
    
	// Rückgabe
	plhs[MEXAD_OUT_Y] = mxCreateDoubleMatrix(m, d+1, mxREAL);
	ptrOutput = mxGetPr(plhs[MEXAD_OUT_Y]);

	

	//	Aufruf der Berechnungsprozedur
	double** pX = myalloc2(n, d+1);
	double** pY = myalloc2(m, d+1);

	madVector2Matrix(ptrInput, pX, n, d+1);
	
	forward(TapeID, m, n, d, keep, pX, pY);
	
	madMatrix2Vector(pY, ptrOutput, m, d+1);
    
	myfree2(pX);
	myfree2(pY);

    return;   
}


