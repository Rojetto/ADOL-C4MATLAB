// madLiebracket.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "adolc\lie\drivers.h"




// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_TAPE_G      1
#define MEXAD_IN_X			 2
#define MEXAD_IN_d           3


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_L          0


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



/* **************************************************************************************
 * *****	Übergabeteil / Gateway-Routine											*****
 * *****	==============================											*****
 * *****																			*****
 * *****	Programmeinsprungpunkt													*****
 * *****																			*****
 * *****	Aufruf in MATLAB: L = madLiebracket(TapeID_F, TapeId_G, X, d)			*****
 * *****																			*****
 * **************************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* ptrOutput;					// Zeiger auf die Rückgabematrix
	double* pX;					// Zeiger auf Werte der 
	
	MexADCTagType TapeID_F;				// Tape-Kennzeichner
	MexADCTagType TapeID_G;
    int m_F;								// Anzahl der abh. Variablen
	int n_F;								// Anzahl der unabh. Variablen
	int m_G;								// Anzahl der abh. Variablen
	int n_G;								// Anzahl der unabh. Variablen
	int d;								// Ableitungsordnung (degree of derivation)


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
	size_t TapeInfo_F[STAT_SIZE];

	size_t TapeInfo_G[STAT_SIZE];
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 4, 4);
	madCheckNumOutputs(nlhs, 0, 1);
            

    // Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId_F")) return; 
	TapeID_F     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]); 
    
    //	Anzahl der "n_F" unabhängigen und "m_F" abhängigen Variablen des Tapes_F
	tapestats(TapeID_F, TapeInfo_F);
	n_F = TapeInfo_F[0];
    m_F = TapeInfo_F[1];

	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_G, "TapeId_G")) return; 
	TapeID_G     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_G]); 
    
    //	Anzahl der "n_G" unabhängigen und "m_G" abhängigen Variablen des Tapes_G
	tapestats(TapeID_G, TapeInfo_G);
	n_G = TapeInfo_G[0];
    m_G = TapeInfo_G[1]; 

	if ( (n_F != m_F) || (m_F != m_G) )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent and independent variables of f must be identical to the number of dependent variables of g!");
    }
	if ( (n_F != m_F) || (n_F != n_G) )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent and independent variables of f must be identical to the number of independent variables of g!");
    }

    // Ableitungsordnung
	if (!CheckIfScalar(prhs, MEXAD_IN_d, "d")) return;
    d = (int)mxGetScalar(prhs[MEXAD_IN_d]);
	
	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n_F, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);
    
	// Rückgabe
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(n_F, d+1, mxREAL);
	ptrOutput = mxGetPr(plhs[MEXAD_OUT_L]);

	

	//	Aufruf der Berechnungsprozedur
	double** pL = myalloc2(n_F, d+1);

	lie_bracket(TapeID_F, TapeID_G, n_F, pX, d, pL);
	
	madMatrix2Vector(pL, ptrOutput, n_F, d+1);
    
	myfree2(pL);
      

    return;   
} 