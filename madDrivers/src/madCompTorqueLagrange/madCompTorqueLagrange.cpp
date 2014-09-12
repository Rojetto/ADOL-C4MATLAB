// madCompTorqueLagrange.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "matrix_h"

using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_L		0
#define MEXAD_IN_X			1
#define MEXAD_IN_Kp			2
#define MEXAD_IN_Kd			3
#define MEXAD_IN_ref		4

// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_L			0
#define MEXAD_OUT_M			1
#define MEXAD_OUT_C			2
#define MEXAD_OUT_VM		3


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


/* **********************************************************************************************
 * *****	Übergabeteil / Gateway-Routine													*****
 * *****	==============================													*****
 * *****																					*****
 * *****	Programmeinsprungpunkt															*****
 * *****																					*****
 * *****	Aufruf in MATLAB: L = madCompTorqueLagrange(TapeID, X, Kp, Kd, ref)		    	*****
 * *****					  [L, M] = madCompTorqueLagrange(TapeID, X, Kp, Kd, ref)		*****
 * *****					  [L, M, C] = madCompTorqueLagrange(TapeID, X, Kp, Kd, ref)		*****
 * *****					  [L, M, C, VM] = madCompTorqueLagrange(TapeID, X, Kp, Kd, ref)	*****
 * *****																					*****
 * **********************************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* pL;					// Zeiger auf die Rückgabematrix
	double* pC;					// Zeiger auf die Rückgabematrix
	double* pM;					// Zeiger auf die Rückgabematrix
	double* pVM;				// Zeiger auf die Rückgabematrix
	double* pX;					// Zeiger auf die Matrix für Taylor-Koeff.
								// der unabh. Variablen
	//double* u;                  // Zeiger auf die Matrix der äußeren Kräfte
	double* Kp;					// Zeiger auf der Verstärkungsmatrix Kp
	double* Kd;					// Zeiger auf der Verstärkungsmatrix Kd
	double* pRef;				// Zeiger auf den Vektor der Referenztrajektorie
	
	MexADCTagType TapeID;		// Tape-Kennzeichner
	int n;						// Anzahl der unabh. Variablen
	int m;						// Anzahl der abh. Variablen
	int dimq;					// Dimension von generalisierten Koordinaten q; dimq = n/2


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
    size_t TapeInfo[STAT_SIZE]; 
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 5, 5);
	madCheckNumOutputs(nlhs, 0, 4);
            

    // Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_L, "TapeId")) return; 
	TapeID     = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_L]); 
    
    //	Anzahl der "n" unabhängigen und "m" abhängigen Variablen des Tapes
    tapestats(TapeID, TapeInfo);
	n = TapeInfo[0];
	m = TapeInfo[1];

	dimq = n/2;

	if ( m!=1 )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent variables must be 1!");
    }

	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	//// Äußere Kräfte u
	//if (!madCheckDim1c(prhs, MEXAD_IN_u, dimq, "u")) return;
 //   u = mxGetPr(prhs[MEXAD_IN_u]);

	// Verstärkungsmatritzen
	if (!madCheckDim2(prhs, MEXAD_IN_Kp, dimq, dimq, "Kp")) return;
	Kp = mxGetPr(prhs[MEXAD_IN_Kp]);
	if (!madCheckDim2(prhs, MEXAD_IN_Kd, dimq, dimq, "Kd")) return;
	Kd = mxGetPr(prhs[MEXAD_IN_Kd]);

	double** pKp = myalloc2(dimq, dimq);
	double** pKd = myalloc2(dimq, dimq);

	madVector2Matrix(Kp, pKp, dimq, dimq);
	madVector2Matrix(Kd, pKd, dimq, dimq);

	// Referenztrajektorie
	if (!madCheckDim1c(prhs, MEXAD_IN_ref, 3*dimq, "ref")) return;
	pRef = mxGetPr(prhs[MEXAD_IN_ref]);

   
	// Rückgabe
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(n, 1, mxREAL);
	pL = mxGetPr(plhs[MEXAD_OUT_L]);
	plhs[MEXAD_OUT_M] = mxCreateDoubleMatrix(dimq, dimq, mxREAL);
	pM = mxGetPr(plhs[MEXAD_OUT_M]);
	plhs[MEXAD_OUT_C] = mxCreateDoubleMatrix(dimq, 1, mxREAL);
	pC = mxGetPr(plhs[MEXAD_OUT_C]);
	plhs[MEXAD_OUT_VM] = mxCreateDoubleMatrix(dimq, 1, mxREAL);
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

	double** pHb = myalloc2(dimq, dimq);			// Hesseteilmatrix B
	double** pHi = myalloc2(dimq, dimq);			// inverse Hesseteilmatrix D
	double** pMh = myalloc2(dimq, dimq);
	double* pZw = myalloc(dimq);					// Zwischenergebnis
	matrix <double> pHd(dimq,dimq);					// Hesseteilmatrix D
	bool xyz;
	double* pu = myalloc(dimq);						// Vektor der Stellmomente
	
	// Initialisierung von Zwischenwerten									
	for (int i = 0 ; i < dimq; i++)
	{
		pZw[i] = 0;
		pL[i+dimq] = 0;
	}

	// Einlesen der Hesseteilmatrix D 
	for (int i = 0; i < dimq; i++)				
	{
		for (int j = 0; j < dimq; j++)
		{
			pHd.setvalue(i,j,pH[i+dimq][j+dimq]);
		}
	}
	
	// Einlesen der Hesseteilmatrix B
	for (int i = 0; i < dimq; i++)				
	{
		for (int j = 0; j < dimq; j++)
		{
			pHb[i][j] = pH[i][j+dimq];
		}
	}

	if(n==2)
	{
		pu[0] = pH[1][1]*( pRef[2] + pKd[0][0]*( pRef[1]-pX[1] ) + pKp[0][0]*( pRef[0]-pX[0] ) );
		
		for (int i = 0; i < dimq; i++)				
		{
			pL[i] = pX[i+dimq];
			pL[i+dimq] = (pow(pH[1][1],-1))*(pJ[0][0] - pH[0][1]*pX[1] + pu[0]);
		}

		pM[0]=pH[1][1];
		pC[0]=pJ[0][0] - pH[0][1]*pX[1];
		pVM[0]=pJ[0][1];
	}

	else
	{
		// Invertierung der Hesseteilmatrix D
		pHd.invert();
	
	
		// Speichern der invertierten Matrix in pHi
		for (int i = 0; i < dimq; i++)				
		{
			for (int j = 0; j < dimq; j++)
			{
				pHd.getvalue(i,j,pHi[i][j],xyz);
			}
		}

		for (int i = 0; i < dimq; i++)				
		{
			for (int j = 0; j < dimq; j++)
			{
				pMh[i][j] = pH[i+dimq][j+dimq];
			}
		}

		madMatrix2Vector(pMh, pM, dimq, dimq);
	       
		// Berechnung von (d^2)L/((dq)(dq_punkt))*(dq_punkt)
		for (int i = 0; i < dimq; i++)				
		{
			for (int j = 0; j < dimq; j++)
			{
				pZw[i] += pHb[j][i]*pX[j+dimq];
			}
		}
	
		// Berechnung von C(q,dq)
		for (int j = 0; j < dimq; j++)
		{
			pC[j] = pZw[j] - pJ[0][j];
		}

		// Verallgemeinerten Momente
		for (int j = 0; j < dimq; j++)
		{
			pVM[j] = pJ[0][j+dimq];			
		}

		// q_punkt
		for (int i = 0; i < dimq; i++)				
		{
			pL[i] = pX[i+dimq];
		}


		// Berechnung der Stellmomente (Computed Torque)
		double* pKddqKpq = myalloc(dimq);

		for (int i = 0; i < dimq; i++)
		{
			for (int j = 0; j < dimq; j++)
			{
				pKddqKpq[i] += pKd[j][i]*(pRef[j+dimq]-pX[j+dimq]) + pKp[j][i]*(pRef[j]-pX[j]);
			}
		}

		for (int i = 0; i < dimq; i++)
		{
			for (int j = 0; j < dimq; j++)
			{
				pu[i] += pMh[i][j]*(pRef[j+n] + pKddqKpq[j]);
			}
		}

		myfree(pKddqKpq);


		// Berechnung von q_2punkt
		for (int i = 0; i < dimq; i++)				
		{
			for (int j = 0; j < dimq; j++)
			{
				pL[i+dimq] += pHi[i][j] * (pJ[0][j] - pZw[j] + pu[j]);			//u[j] besteht aus der Summe der äußeren Kräfte und den Dissipationskräften
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

	myfree2(pKp);
	myfree2(pKd);
	myfree(pu);
	
    return;   
}
