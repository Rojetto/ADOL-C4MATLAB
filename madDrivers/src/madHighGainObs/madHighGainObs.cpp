// madHighGainObs.cpp : Definiert die exportierten Funktion forward(tag,m,n,d,keep,X,Y) für die Einbindung als DLL.


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "adolc\lie\drivers.h"



// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_TAPE_H      1
#define MEXAD_IN_X			 2
#define MEXAD_IN_K			 3
#define MEXAD_IN_KAPPA		 4


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_KHG        0


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
 * *****	Aufruf in MATLAB: kHG = madHighGainObs(TapeID_F, TapeID_H, X, k)		*****
 * *****					  KHG = madHighGainObs(TapeID_F, TapeID_H, X, K, kappa)	*****
 * *****																			*****
 * **************************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* pKHG;						// Zeiger auf die Rückgabematrix
	double* pX;							// Zeiger auf den Vektor der unabh. Variablen
	double* pK;							// Zeiger auf die Verstärkungsmatrix
	double* pKappa;						// Zeiger auf den Vektor der Speudobeobachtbarkeitsindizes
	
	MexADCTagType TapeID_F;				// Tape-Kennzeichner Vektorfeld f
	MexADCTagType TapeID_H;				// Tape-Kennzeichner Skalarfeld/Vektorfeld h
    int m_F;							// Anzahl der abh. Variablen
	int n_F;							// Anzahl der unabh. Variablen
    int m_H;							// Anzahl der abh. Variablen
	int n_H;							// Anzahl der unabh. Variablen



	//	Arrays mit Informationen zu den Tapes (siehe AdolC\taping.c, Funktion tapestats())
    size_t TapeInfo_F[STAT_SIZE]; 
	size_t TapeInfo_H[STAT_SIZE];
           

	// Initialisierung der MEX-Funktion
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);
    

    // Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 4, 5);
	madCheckNumOutputs(nlhs, 0, 1);
            

    // Tape_ID_F ermitteln und zugehörige Informationen des Tapes in Array TapeInfo_F speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId_F")) return; 
		TapeID_F = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]); 
    
    //	Anzahl der "n_F" unabhängigen und "m_F" abhängigen Variablen des Tapes F
    tapestats(TapeID_F, TapeInfo_F);
	n_F = TapeInfo_F[0];
    m_F = TapeInfo_F[1];  


    // Tape_ID_H ermitteln und zugehörige Informationen des Tapes in Array TapeInfo_H speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_H, "TapeId_H")) return; 
		TapeID_H = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_H]); 
    
    //	Anzahl der "n_H" unabhängigen und "m_H" abhängigen Variablen des Tapes H
    tapestats(TapeID_H, TapeInfo_H);
	n_H = TapeInfo_H[0];
    m_H = TapeInfo_H[1];

	if ( (n_F != m_F) || (n_F != n_H) )
    {
       mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),  
       "The number of dependent and independent variables of f must be identical to the number of independent variables of h!");
    }
	

	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n_F, "X")) return;
		pX = mxGetPr(prhs[MEXAD_IN_X]);


	// k bzk K und kappa
	if (nrhs == 4)	// SISO
	{
		if (!madCheckDim1c(prhs, MEXAD_IN_K, n_F, "k")) return;
			pK = mxGetPr(prhs[MEXAD_IN_K]);
	}
	else			// MIMO
	{
		if (!madCheckDim2(prhs, MEXAD_IN_K, n_F, m_H, "K")) return;
			pK = mxGetPr(prhs[MEXAD_IN_K]);
		if (!madCheckDim1c(prhs, MEXAD_IN_KAPPA, m_H, "kappa")) return;
			pKappa = mxGetPr(prhs[MEXAD_IN_KAPPA]);
	}

    

	// Rückgabe
	plhs[MEXAD_OUT_KHG] = mxCreateDoubleMatrix(n_F, 1, mxREAL);
	pKHG = mxGetPr(plhs[MEXAD_OUT_KHG]);


	double** pQS = myalloc2(n_F,n_F);
	

	//	Aufruf der Berechnungsprozedur
	if(m_H==1)	// SISO
	{
		//	Aufruf der Berechnungsprozedur
		double** pQT = myalloc2(n_F, n_F);

		lie_gradient(TapeID_F, TapeID_H, n_F, pX, n_F-1, pQT);
		
		// Matrix transponieren, da lie_gradient die transonierte Beobachtbarkeitsmatrix liefert
		for (int i=0; i<n_F; i++)
		{
			for (int j=0; j<n_F; j++)
			{
				pQS[i][j] = pQT[j][i];
			}
		}
		myfree2(pQT);

	}
	else	// MIMO
	{
		int kappa_max = 0;
		int kappa_sum = 0;

		for (int i=0; i<m_H; i++)
		{
			if (pKappa[i] > kappa_max)
				kappa_max = (int)pKappa[i];
			kappa_sum += (int)pKappa[i];
		}

		if (kappa_sum != n_F)
			mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch), "The sum of the elements of kappa must be equal to the number of independent variables!");
		
		double*** pQRT = myalloc3(m_H, n_F, kappa_max);
		double** pQR  = myalloc2(m_H*kappa_max, n_F);

		lie_gradient(TapeID_F, TapeID_H, n_F, m_H, pX, kappa_max-1, pQRT);

		
		// reduzierte Steuerbarkeitsmatrix zusammen setzen
		for (int i = 0; i < m_H; i++)
		{
			for (int j = 0; j < n_F; j++)
			{
				for (int k = 0; k < kappa_max; k++)
				{
					pQR[i+k*(m_H)][j] = pQRT[i][j][k];
				};
			};
		};


		// Matrix P
		double** pP = myalloc2(n_F, kappa_max*m_H);
		int row=0;
		for (int i=0; i<m_H; i++)
		{
			for (int j=0; j<pKappa[i]; j++)
			{
				pP[row++][i+j*m_H] = 1;
			}
		}


		// Auswahlmatrix berechnen
		for (int i=0; i<n_F; i++)
		{
			for (int j=0; j<n_F; j++)
			{
				for (int k=0; k<kappa_max*m_H; k++)
				{
					pQS[i][j] += pP[i][k]*pQR[k][j];
				}
			}
		}
		myfree2(pP);
		myfree3(pQRT);
		myfree2(pQR);
	}


	mxArray *param[2];
	param[0] = mxCreateDoubleMatrix(n_F, n_F, mxREAL);
	param[1] = mxCreateDoubleMatrix(n_F, m_H, mxREAL);
	double *pparam1;
	double *pparam2;
	pparam1 = mxGetPr(param[0]);
	pparam2 = mxGetPr(param[1]);

	madMatrix2Vector(pQS, pparam1, n_F, n_F);
	for (int i=0; i<n_F*m_H; i++)
	{
		pparam2[i] = pK[i];
	}
	
	mexCallMATLAB(1, plhs, 2, param, "mldivide");

	mxDestroyArray(param[0]);
	mxDestroyArray(param[1]);

	myfree2(pQS);

    return;   
}


