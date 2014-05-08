// madLagrange.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madhelpers.h"
#include "invMat.h"

using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_X			 1
#define MEXAD_IN_u	         2


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
	double* pX;					// Zeiger auf die Matrix für Taylor-Koeff.
								// der unabh. Variablen
	double* u;                  // Zeiger auf die Matrix der äußeren Kräfte
	
	MexADCTagType TapeID;				// Tape-Kennzeichner
	int n;								// Anzahl der unabh. Variablen
	int m;								// Anzahl der abh. Variablen


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
    int TapeInfo[STAT_SIZE]; 
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 3, 3);
	madCheckNumOutputs(nlhs, 0, 1);
            

    // Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId")) return; 
	TapeID     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]); 
    
    //	Anzahl der "n" unabhängigen und "m" abhängigen Variablen des Tapes
	tapestats(TapeID, (size_t *)TapeInfo);
	n = TapeInfo[0];
	m = TapeInfo[1];

	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	// Äußere Kräfte u
	if (!madCheckDim1c(prhs, MEXAD_IN_u, n/2, "u")) return;
    u = mxGetPr(prhs[MEXAD_IN_u]);
   
	// Rückgabe
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(n, 1, mxREAL);
	pL = mxGetPr(plhs[MEXAD_OUT_L]);

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

	double** pHd = myalloc2(n/2, n/2);			// Hesseteilmatrix D
	double** pHb = myalloc2(n/2, n/2);			// Hesseteilmatrix B
	double** pHi = myalloc2(n/2, n/2);			// inverse Hesseteilmatrix D
	double* pZw = myalloc(n/2);					// Zwischenergebnis
	
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
			pHd[i][j] = pH[i+n/2][j+n/2];
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

	//Berechnung der Unterdeterminanten
	double** hilfsMb = myalloc2(n/2, n/2);
	double** fac = myalloc2(n/2, n/2);
	double** hilfsMc = myalloc2(n/2, n/2);
	double d;
    int o, l, i, j, k;
 
	for (int q = 0; q < n/2; q++)
	{
		for (int p = 0; p < n/2; p++)
		{
			o = 0;
			l = 0;

			for (int i = 0; i < n/2; i++)
			{
				for (int j = 0; j < n/2; j++)
				{
					hilfsMb[i][j] = 0;

					if (i != q && j != p)
					{
						hilfsMb[o][l] = pHd[i][j];

						if (l < (n/2 - 2))
							l++;
						else
						{
							l = 0;
							o++;
						}
					}
				}
			}
			k = n/2 - 1;
			fac[q][p] = pow(-1.0, q + p)*detrm(hilfsMb, k);
		}
	}

	//Berechnung der inversen Hesseteilmatrix D
	for (i = 0; i < n/2; i++)
	{
		for (j = 0; j < n/2; j++)
		{
			hilfsMc[i][j] = fac[j][i];
		}
	}

	d = detrm(pHd, n/2);

	for (i = 0; i < n/2; i++)
	{
		for (j = 0; j < n/2; j++)
		{
			pHi[i][j] = 0;
		}
	}

	for (int i = 0; i < n/2; i++)
	{
		for (int j = 0; j < n/2; j++)
		{
			pHi[i][j] = hilfsMc[i][j]/d;
		}
	}


	// Berechnung von (d^2)L/((dq)(dq_punkt))*(dq_punkt)
	for (int i = 0; i < n/2; i++)				
	{
		for (int j = 0; j < n/2; j++)
		{
			pZw[i] = pZw[i] + pHb[i][j]*pX[j+n/2];
		}
	}
	
	// q_punkt
	for (int i = 0; i < n/2; i++)				
	{
		pL[i] = pX[i+n/2];
	};

	// Berechnung von q_2punkt
	for (int i = 0; i < n/2; i++)				
	{
		for (int j = 0; j < n/2; j++)
		{
			pL[i+n/2] = pL[i+n/2] + pHi[i][j]*(pJ[0][j] - pZw[j] + u[j]);
		}
	}

	myfree2(pJ);
	myfree2(pH);
	myfree2(pHd);
	myfree2(pHb);
	myfree2(pHi);
	myfree(pZw);
	myfree2(hilfsMb);
	myfree2(hilfsMc);
	myfree2(fac);


    return;   
}
