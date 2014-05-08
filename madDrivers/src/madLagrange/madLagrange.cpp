// madLagrange.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "matrix_h"

using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_X			 1
#define MEXAD_IN_u	         2


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_L          0
#define MEXAD_OUT_M			 1
#define MEXAD_OUT_C			 2
#define MEXAD_OUT_VM		 3


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
 * *****	Aufruf in MATLAB: L = madLagrange(TapeID, X, u)		    	*****
 * *****																*****
 * **************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* pL;					// Zeiger auf die Rückgabematrix
	double* pC;					// Zeiger auf die Rückgabematrix
	double* pM;					// Zeiger auf die Rückgabematrix
	double* pVM;					// Zeiger auf die Rückgabematrix
	double* pX;					// Zeiger auf die Matrix für Taylor-Koeff.
								// der unabh. Variablen
	double* u;                  // Zeiger auf die Matrix der äußeren Kräfte
	
	MexADCTagType TapeID;				// Tape-Kennzeichner
	int n;								// Anzahl der unabh. Variablen
	int m;								// Anzahl der abh. Variablen


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
    size_t TapeInfo[STAT_SIZE]; 
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 3, 3);
	madCheckNumOutputs(nlhs, 0, 4);
            

    // Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId")) return; 
	TapeID     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]); 
    
    //	Anzahl der "n" unabhängigen und "m" abhängigen Variablen des Tapes
    tapestats(TapeID, TapeInfo);
	n = TapeInfo[0];
	m = TapeInfo[1];

	if ( m!=1 )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent variables must be 1!");
    }

	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	// Äußere Kräfte u
	if (!madCheckDim1c(prhs, MEXAD_IN_u, n/2, "u")) return;
    u = mxGetPr(prhs[MEXAD_IN_u]);
   
		// Rückgabe
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(n, 1, mxREAL);
	pL = mxGetPr(plhs[MEXAD_OUT_L]);
	plhs[MEXAD_OUT_M] = mxCreateDoubleMatrix(n/2, n/2, mxREAL);
	pM = mxGetPr(plhs[MEXAD_OUT_M]);
	plhs[MEXAD_OUT_C] = mxCreateDoubleMatrix(n/2, 1, mxREAL);
	pC = mxGetPr(plhs[MEXAD_OUT_C]);
	plhs[MEXAD_OUT_VM] = mxCreateDoubleMatrix(n/2, 1, mxREAL);
	pVM = mxGetPr(plhs[MEXAD_OUT_VM]);

	//	Aufruf der Berechnungsprozedur

	double** pJ = myalloc2(m, n);
	jacobian(TapeID, m, n, pX, pJ);

	double** pH = myalloc2(n, n);				// pH = (A B; C D)
	hessian(TapeID, n, pX, pH);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j <= i-1; j++)
		{
			pH[j][i] = pH[i][j];
		};
	};

	double** pHb = myalloc2(n/2, n/2);			// Hesseteilmatrix B
	double** pHi = myalloc2(n/2, n/2);			// inverse Hesseteilmatrix D
	double** pMh = myalloc2(n/2, n/2);
	double* pZw = myalloc(n/2);					// Zwischenergebnis
	matrix <double> pHd(n/2,n/2);				// Hesseteilmatrix D
	bool xyz;
	
	// Initialisierung von Zwischenwerten
	pZw[0] = 0;									
	
	for (int i = 0 ; i < n/2; i++)
	{
		pL[i+n/2] = 0;
	}

	// Einlesen der Hesseteilmatrix D 
	for (int i = 0; i < n/2; i++)				
	{
		for (int j = 0; j < n/2; j++)
		{
			pHd.setvalue(i,j,pH[i+n/2][j+n/2]);
		}
	}
	
	// Einlesen der Hesseteilmatrix B
	for (int i = 0; i < n/2; i++)				
	{
		for (int j = 0; j < n/2; j++)
		{
			pHb[i][j] = pH[i][j+n/2];
		}
	}

	if(n==2)
	{
		for (int i = 0; i < n/2; i++)				
		{
			pL[i] = pX[i+n/2];
			pL[i+n/2] = (pow(pH[1][1],-1))*(pJ[0][0] - pH[0][1]*pX[1] + u[0]);
		}

		pM[0]=pH[1][1];
		pC[0]=pJ[0][0] - pH[0][1]*pX[1];
		pVM[0]=pJ[0][1];
	}

	// Invertierung der Hesseteilmatrix D
	else
	{
		pHd.invert();
	
	
		// Speichern der invertierten Matrix in pHi
		for (int i = 0; i < n/2; i++)				
		{
			for (int j = 0; j < n/2; j++)
			{
				pHd.getvalue(i,j,pHi[i][j],xyz);
			}
		}

		for (int i = 0; i < n/2; i++)				
		{
			for (int j = 0; j < n/2; j++)
			{
				pMh[i][j] = pH[i+n/2][j+n/2];
			}
		}

		madMatrix2Vector(pMh, pM, n/2, n/2);
	       
		// Berechnung von (d^2)L/((dq)(dq_punkt))*(dq_punkt)
		for (int i = 0; i < n/2; i++)				
		{
			for (int j = 0; j < n/2; j++)
			{
				pZw[i] = pZw[i] + pHb[j][i]*pX[j+n/2];
			}
		}
	
		// Berechnung von C(q,dq)
		for (int j = 0; j < n/2; j++)
		{
			pC[j] = -(pC[j] + (pJ[0][j] - pZw[j]));			
		}

		// Verallgemeinerten Momente
		for (int j = 0; j < n/2; j++)
		{
			pVM[j] = pJ[0][j+n/2];			
		}

		// q_punkt
		for (int i = 0; i < n/2; i++)				
		{
			pL[i] = pX[i+n/2];
		}

		// Berechnung von q_2punkt
		for (int i = 0; i < n/2; i++)				
		{
			for (int j = 0; j < n/2; j++)
			{
				pL[i+n/2] = pL[i+n/2] + pHi[i][j]*(pJ[0][j] - pZw[j] + u[j]);			//u[j] besteht aus der Summe der äußeren Kräfte und den Dissipationskräften
			}
		}
	}

	//Freigabe des Speichers
	myfree2(pJ);
	myfree2(pH);
	myfree2(pHb);
	myfree2(pHi);
	myfree2(pMh);
	myfree(pZw);
	
    return;   
}
