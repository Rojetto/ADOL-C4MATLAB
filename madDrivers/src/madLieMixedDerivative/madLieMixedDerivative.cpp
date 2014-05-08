// madLiegemischt.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "adolc_lie.h"


using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_TAPE_FG	 1
#define MEXAD_IN_TAPE_H      2
#define MEXAD_IN_X			 3
#define MEXAD_IN_d           4


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_L          0


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



/* ******************************************************************************************
 * *****	Übergabeteil / Gateway-Routine												*****
 * *****	==============================												*****
 * *****																				*****
 * *****	Programmeinsprungpunkt														*****
 * *****																				*****
 * *****	Aufruf in MATLAB: L = madLiegemischt(TapeID_F, TapeID_FG, TapeId_H, X, d)	*****
 * *****																				*****
 * ******************************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* ptrOutput;					// Zeiger auf die Rückgabematrix
	double* pX;					// Zeiger auf Werte der 
	
	MexADCTagType TapeID_F;				// Tape-Kennzeichner
	MexADCTagType TapeID_FG;
	MexADCTagType TapeID_H;
    int m_F;								// Anzahl der abh. Variablen
	int n_F;								// Anzahl der unabh. Variablen
	int m_FG;
	int n_FG;
	int m_H;								// Anzahl der abh. Variablen
	int n_H;								// Anzahl der unabh. Variablen
	int d;								// Ableitungsordnung (degree of derivation)


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
    size_t TapeInfo_F[STAT_SIZE]; 

	size_t TapeInfo_FG[STAT_SIZE];

	size_t TapeInfo_H[STAT_SIZE];
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 5, 5);
	madCheckNumOutputs(nlhs, 0, 1);
            

    // Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId_F")) return; 
	TapeID_F     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]); 
    
    //	Anzahl der "n_F" unabhängigen und "m_F" abhängigen Variablen des Tapes_F
    tapestats(TapeID_F, TapeInfo_F);
	n_F = TapeInfo_F[0];
    m_F = TapeInfo_F[1];

	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_FG, "TapeId_FG")) return; 
	TapeID_FG     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_FG]); 
    
    //	Anzahl der "n_F" unabhängigen und "m_F" abhängigen Variablen des Tapes_F
    tapestats(TapeID_FG, TapeInfo_FG);
	n_FG = TapeInfo_FG[0];
    m_FG = TapeInfo_FG[1];

	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_H, "TapeId_H")) return; 
	TapeID_H     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_H]); 
    
    //	Anzahl der "n_H" unabhängigen und "m_H" abhängigen Variablen des Tapes_H
    tapestats(TapeID_H, TapeInfo_H);
	n_H = TapeInfo_H[0];
    m_H = TapeInfo_H[1]; 

	if ( (n_F != m_F) || (n_F != n_H) )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent and independent variables of f must be identical to the number of independent variables of h!");
    }
	if ( (n_FG != m_FG) || (n_FG != n_H) )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent and independent variables of f must be identical to the number of independent variables of h!");
    }

    // Ableitungsordnung
	if (!CheckIfScalar(prhs, MEXAD_IN_d, "d")) return;
    d = (int)mxGetScalar(prhs[MEXAD_IN_d]);
	
	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n_F, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);
    

	if(m_H == 1)
	{
	double* pL1 = myalloc(d+1);
	double* pL2 = myalloc(d+1);
	// Rückgabe
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(d+1, 1, mxREAL);
	ptrOutput = mxGetPr(plhs[MEXAD_OUT_L]);

		//	Aufruf der Berechnungsprozedur	

	lie_scalar(TapeID_F, TapeID_H, n_F, pX, d, pL1);

	lie_scalar(TapeID_FG, TapeID_H, n_FG, pX, d, pL2);

	for(int i=0; i<d+1; i++)
	{
		ptrOutput[i] = pL2[i]-pL1[i];
	}
	myfree(pL1);
	myfree(pL2);
	}

	else
	{
	double** pL = myalloc2(m_H, d+1);
	double** pL3 = myalloc2(m_H, d+1);
	double** pL4 = myalloc2(m_H, d+1);

	// Rückgabe
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(m_H, d+1, mxREAL);
	ptrOutput = mxGetPr(plhs[MEXAD_OUT_L]);

	//	Aufruf der Berechnungsprozedur

	lie_scalar(TapeID_F, TapeID_H, n_F, m_H, pX, d, pL3);
	lie_scalar(TapeID_FG, TapeID_H, n_FG, m_H, pX, d, pL4);

	for(int i=0; i<m_H; i++)
	{
		for(int j=0; j<d+1; j++)
		{
			pL[i][j] = pL4[i][j]-pL3[i][j];			
		}
	}

	madMatrix2Vector(pL, ptrOutput, m_H, d+1);

	myfree2(pL);
	myfree2(pL3);
	myfree2(pL4);
	}   

    return; 
}