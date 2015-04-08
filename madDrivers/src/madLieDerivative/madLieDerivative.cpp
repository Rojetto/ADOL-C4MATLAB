// madLieDerivative


// Benötigte Header
#include "mex.h"
#include "adolc/adolc.h"
#include "madHelpers.h"

// Position und Bedeutung der Eingabeparameter der Mex-Funktion
#define MEXAD_IN_TapeF        0
#define MEXAD_IN_TapeH        1
#define MEXAD_IN_x            2
#define MEXAD_IN_d            3

// Position und Bedeutung der Rückgabewerte
#define MEXAD_OUT_L           0

using namespace std;

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



/* **************************************************************************************
* *****	Übergabeteil / Gateway-Routine												*****
* *****	==============================												*****
* *****																				*****
* *****	Programmeinsprungpunkt														*****
* *****																				*****
* *****	Aufruf in MATLAB: L = madLieDerivative(TapeID_F, TapeId_H, X, d)			*****
* *****																				*****
* **************************************************************************************
*/
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray*prhs[] )
     
{ 
    double* pL;       // Lie-Ableitungen
    double* pX; 
    MexADCTagType Tape_F, Tape_H;
    int n_F, m_F, n_H, m_H, nInput, mInput, d;
    size_t TapeInfo_F[STAT_SIZE], TapeInfo_H[STAT_SIZE];
           
	// Initialisierung der Mex-Funktion
    if (!MexInitialized) 
    {
        mexPrintf("Initializing %s\n", __FILE__);
        
        // Persistenz sichern
        persistent_array_ptr = mxCreateDoubleMatrix(1, 1, mxREAL);
        mexMakeArrayPersistent(persistent_array_ptr);
        mexAtExit(cleanup);       
        
        // Flag setzen
        MexInitialized = true;
    }    
    
    // Funktion verlangt zwei Parameter (x und d)
    if (nrhs != 4) 
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(TooManyInputs), "Too many inputs"); 
    } 
    
    // Funktion gibt einen Vektor zurück
    if (nlhs > 1) 
    {
	   mexErrMsgIdAndTxt(MEXADC_ErrId(TooManyOutputs), "Too many outputs!"); 
    } 
    
    // Tape_F
    if ( mxGetNumberOfElements(prhs[MEXAD_IN_TapeF]) != 1 ) 
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(FScalar), "Tape_F must be scalar!"); 
    }  
    Tape_F     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TapeF]);
    tapestats(Tape_F, TapeInfo_F);

    // Number of independents n and dependents m of f(x)
    n_F = TapeInfo_F[0];
    m_F = TapeInfo_F[1];
    
       
    // Tape_H
    if ( mxGetNumberOfElements(prhs[MEXAD_IN_TapeH]) != 1 ) 
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(HScalar), "Tape_H must be scalar!"); 
    }  
    Tape_H = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TapeH]);
	tapestats(Tape_H, TapeInfo_H);

    // Number of independents n and dependents m of h(x)
    n_H = TapeInfo_H[0];
    m_H = TapeInfo_H[1];
    
    if ( (n_F != m_F) || (n_F != n_H) )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The tapes recorded belongs to functions f : R^%d -> R^%d and h : R^%d -> R^%d. However, the number of dependent and independent variables of f must be identical to the number of independent variables of h!", 
       n_F, m_F, n_H, m_H);
    }
    
    // x
    mInput = mxGetN(prhs[MEXAD_IN_x]); 
    nInput = mxGetM(prhs[MEXAD_IN_x]);
    if ( !mxIsDouble(prhs[MEXAD_IN_x]) || mxIsComplex(prhs[MEXAD_IN_x]) || 
         (m_H != mInput) || (n_H != nInput) ) 
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(XWrongDim), "X must be a %d x %d matrix!", n_H, m_H); 
    } 
    pX = mxGetPr(prhs[MEXAD_IN_x]); 
    
    
    // d
    if ( mxGetNumberOfElements(prhs[MEXAD_IN_d]) != 1 ) 
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(DScalar), "d must be a scalar!"); 
    }         
    d = (int)mxGetScalar(prhs[MEXAD_IN_d]);

    // Zurückgegeben wird ein reeller d+1 x 1 Spaltenvektor
    plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(d+1, 1, mxREAL); 
    
    // Zeiger auf Lie-Ableitungen = Rückgabezeiger! Achtung! Hier keine
    // Allokation nötig, das wurde bereits durch mxCreateDoubleMatrix gemacht!
    pL = mxGetPr(plhs[MEXAD_OUT_L]);
    
    // Rechnen
    //mexPrintf("CalcLieDeriv(%d, %d, %d, %d, pX, %d, pL)", Tape_F, Tape_H, n, m, d); 
    //CalcLieDeriv(Tape_F, Tape_H, n_H, m_H, pX, d, pL); 

    return;   
}


