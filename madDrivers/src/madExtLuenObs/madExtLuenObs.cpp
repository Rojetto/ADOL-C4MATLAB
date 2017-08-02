// madExtLuenObs.cpp : Definiert die exportierten Funktion forward(tag,m,n,d,keep,X,Y) für die Einbindung als DLL.


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
#define MEXAD_OUT_KEW        0


// wird nach dem 1.Aufruf auf true gesetzt
static bool    MexInitialized = false;   

/* Für Matlab, damit Datei persistent wird - einmalige Zuordnung des File-Descriptors, um
 * Polling auf das Tape zu umgehen 
 */
static mxArray *persistent_array_ptr = NULL;


int binomialCoeff(unsigned int n, unsigned int k);


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
 * *****	Aufruf in MATLAB: kEW = madExtLuenObs(TapeID_F, TapeID_H, X, k)			*****
 * *****					  KWE = madExtLuenObs(TapeID_F, TapeID_H, X, K, kappa)	*****
 * *****																			*****
 * **************************************************************************************
 */
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
	// Variablendefinitionen für die Verwendung 
	double* pKEW;						// Zeiger auf die Rückgabematrix
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
	n_F = TapeInfo_F[NUM_INDEPENDENTS];
    m_F = TapeInfo_F[NUM_DEPENDENTS];  


    // Tape_ID_H ermitteln und zugehörige Informationen des Tapes in Array TapeInfo_H speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_H, "TapeId_H")) return; 
		TapeID_H = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_H]); 
    
    //	Anzahl der "n_H" unabhängigen und "m_H" abhängigen Variablen des Tapes H
    tapestats(TapeID_H, TapeInfo_H);
	n_H = TapeInfo_H[NUM_INDEPENDENTS];
    m_H = TapeInfo_H[NUM_DEPENDENTS];

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
	plhs[MEXAD_OUT_KEW] = mxCreateDoubleMatrix(n_F, m_H, mxREAL);
	pKEW = mxGetPr(plhs[MEXAD_OUT_KEW]);


	
	

	//	Aufruf der Berechnungsprozedur
	if (nrhs == 4)	// SISO
	{
		double** pQS = myalloc2(2 * n_F, n_F);
		double** pQT = myalloc2(n_F, 2*n_F);

		//	erweiterte Beobachtbarkeitsmatrix
		lie_gradient(TapeID_F, TapeID_H, n_F, pX, 2*n_F-1, pQT);
		
		// Matrix transponieren, da lie_gradient die transonierte Beobachtbarkeitsmatrix liefert
		for (int i=0; i<2*n_F; i++)
		{
			for (int j=0; j<n_F; j++)
			{
				pQS[i][j] = pQT[j][i];
			}
		}
		myfree2(pQT);


		mxArray *param[2];
		param[0] = mxCreateDoubleMatrix(n_F, n_F, mxREAL);	// Q[0]
		param[1] = mxCreateDoubleMatrix(n_F, 1, mxREAL);	// e_(n_F) bzw. später temp
		double *pparam;
		pparam = mxGetPr(param[1]);

		mxArray *V[1];
		V[0] = mxCreateDoubleMatrix(n_F, n_F + 1, mxREAL);		// v[]
		double *pVk;
		pVk = mxGetPr(V[0]);
		
		mxArray *Vkpp[1];
		double *pVkpp;
		
		madMatrix2Vector(pQS, mxGetPr(param[0]), n_F, n_F);
		pparam[n_F-1] = 1;
	

		// Startvektor
		mexCallMATLAB(1, Vkpp, 2, param, "mldivide");	// v[0] = -Q[0]\e_(n_F)
		pVkpp = mxGetPr(Vkpp[0]);	// muss nach Aufruf von mldivide aktualisiert werden

		for (int i = 0; i<n_F; i++)
		{
			pVk[i] = pVkpp[i];	// v[0] abspeichern
		}


		// Berechnung der Vektoren v[k]
		int bc;
		for (int k=0; k<n_F; k++)
		{
			for (int l = 0; l < n_F; l++) pparam[l] = 0;
			
			for (int i=0; i<=k; i++) // Summenfunktion
			{
				bc = binomialCoeff(k+1,i+1);
				for (int j=0; j<n_F; j++)	// eizelne Zeilen von v[k+1]
				{
					for (int l=0; l<n_F; l++)	// Matrix-Vektor-Multiplikation
					{
						pparam[l] += bc*pQS[i + 1 + l][j] * pVk[n_F*(k - i) + j];
					}
				}
			}

			mexCallMATLAB(1, Vkpp, 2, param , "mldivide");	// v[k+1] = Q[0]\temp
			pVkpp = mxGetPr(Vkpp[0]);	// muss nach Aufruf von mldivide aktualisiert werden
			
			for (int i=0; i<n_F; i++)
			{
				pVk[n_F*(k + 1) + i] = -pVkpp[i]; // v[k+1] abspeichern und Vorzeichen korrigieren
			}
		}


		// Koeffizienten des char. Polynoms um eine 1 erweitern
		double* pPEW;
		pPEW = myalloc(n_F+1);
		int vz = 1;
		for (int i=0; i<n_F; i++)
		{
			pPEW[i] = pK[i]*vz;
			vz *= -1;
		}
		pPEW[n_F] = vz;


		// Beobachterverstärkung berechnen
		for (int iV=0; iV<n_F; iV++)		// Zeile von V
		{
			for (int jV=0; jV<n_F+1; jV++)	// Spalte von V
			{
				pKEW[iV] += pVk[iV + jV*n_F] * pPEW[jV];
			}
		}


		mxDestroyArray(param[0]);
		mxDestroyArray(param[1]);
		mxDestroyArray(V[0]);
		mxDestroyArray(Vkpp[0]);

		myfree2(pQS);
		myfree1(pPEW);

	}
	else	// MIMO
	{
		int kappa_max = 0;
		int kappa_sum = 0;
		unsigned int *nu = myalloc1_uint(m_H);

		for (int i = 0; i<m_H; i++)
		{
			if (pKappa[i] > kappa_max)
				kappa_max = (int)pKappa[i];
			kappa_sum += (int)pKappa[i];
			nu[i] = kappa_sum;
		}

		if (kappa_sum != n_F)
			mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch), "The sum of the elements of kappa must be equal to the number of independent variables!");

		double*** pQSi = myalloc3(kappa_max+1, n_F, n_F);
		double*** pQRT = myalloc3(m_H, n_F, 2*kappa_max);
		double** pQR = myalloc2(2*m_H*kappa_max, n_F);

		lie_gradient(TapeID_F, TapeID_H, n_F, m_H, pX, 2*kappa_max - 1, pQRT);


		// erweiterte Beobachtbarkeitsmatrix zusammen setzen
		for (int i = 0; i < m_H; i++)
		{
			for (int j = 0; j < n_F; j++)
			{
				for (int k = 0; k < 2*kappa_max; k++)
				{
					pQR[i + k*(m_H)][j] = pQRT[i][j][k];
				};
			};
		};


		// Matrix P
		double** pP = myalloc2(n_F, kappa_max*m_H);
		int row = 0;
		for (int i = 0; i<m_H; i++)
		{
			for (int j = 0; j<pKappa[i]; j++)
			{
				pP[row++][i + j*m_H] = 1;
			}
		}


		// Auswahlmatrix Q[0] und weitere Q[l] berechnen
		for (int l = 0; l <= kappa_max; l++)
		{
			for (int i = 0; i < n_F; i++)
			{
				for (int j = 0; j < n_F; j++)
				{
					pQSi[l][i][j] = 0;
					for (int k = 0; k < kappa_max*m_H; k++)
					{
						pQSi[l][i][j] += pP[i][k] * pQR[k+l*m_H][j];
					}
				}
			}
		}


		mxArray *param1[2];
		param1[0] = mxCreateDoubleMatrix(n_F, n_F, mxREAL);		// Q[0]
		param1[1] = mxCreateDoubleMatrix(n_F, m_H, mxREAL);		// e_(nu_i) bzw. später temp
		double *pparam12;
		pparam12 = mxGetPr(param1[1]);

		mxArray *V[1];
		V[0] = mxCreateDoubleMatrix(n_F, n_F + m_H, mxREAL);	// v[ , ]
		double *pVk;
		pVk = mxGetPr(V[0]);

		mxArray *Vkpp[1];
		double *pVkpp;

		madMatrix2Vector(pQSi[0], mxGetPr(param1[0]), n_F, n_F);	// Q[0]
		for (int i = 0; i < m_H; i++)
		{
			pparam12[i*n_F+nu[i]-1] = 1;							// e_(nu_i)
		}


		// Startvektor
		mexCallMATLAB(1, Vkpp, 2, param1, "mldivide");	// v[i,0] = -Q[0]\e_(nu_i)
		pVkpp = mxGetPr(Vkpp[0]);	// muss nach Aufruf von mldivide aktualisiert werden

		for (int i = 0; i < n_F; i++)
		{
			for (int j = 0; j < m_H; j++)
			{
				pVk[i*(n_F+m_H)+(j+nu[j]-(int)pKappa[j])] = pVkpp[i+j*n_F];	// v[i,0] abspeichern
			}
		}


		// Berechnung der Vektoren v[i,k]
		int bc;
		for (int k = 0; k < kappa_max; k++)
		{
			for (int l = 0; l < n_F*m_H; l++) 
				pparam12[l] = 0;

			for (int s = 0; s <= k; s++) // Summenfunktion
			{
				bc = binomialCoeff(k + 1, s + 1);
				for (int i = 0; i < m_H; i++)			// Spalten von pparam12
				{
					if (k < pKappa[i])
					{
						for (int j = 0; j < n_F; j++)		// Zeilen von pparam12
						{
							for (int l = 0; l < n_F; l++)	// Matrix-Vektor-Multiplikation
							{
								pparam12[i*n_F + j] += bc*pQSi[s + 1][j][l] * pVk[l*(n_F + m_H) + (k - s + i + nu[i] - (int)pKappa[i])];
							}
						}
					}
				}
			}

			mexCallMATLAB(1, Vkpp, 2, param1, "mldivide");	// v[k+1] = Q[0]\temp
			pVkpp = mxGetPr(Vkpp[0]);	// muss nach Aufruf von mldivide aktualisiert werden

			for (int i = 0; i < n_F; i++)
			{
				for (int j = 0; j < m_H; j++)
				{
					if (k < pKappa[j])
					{
						pVk[i*(n_F + m_H) + (k + 1 + j + nu[j] - (int)pKappa[j])] = -pVkpp[i + j*n_F];	// v[j,k] abspeichern
					}
				}
			}
		}



		// Koeffizienten-Matrix
		double** pPEW = myalloc2(n_F + m_H, m_H);
		int vz;
		for (int j = 0; j < m_H; j++)
		{
			vz = 1;
			for (int i = 0; i < pKappa[j]; i++)
			{
				pPEW[i + nu[j] - (int)pKappa[j] + j][j] = pK[i + nu[j] - (int)pKappa[j] + j*n_F] * vz;
				vz *= -1;
			}
			pPEW[nu[j] + j][j] = vz;
		}
		

		double** pKELi = myalloc2(n_F, m_H);
		for (int i = 0; i < n_F; i++)
		{
			for (int j = 0; j < m_H; j++)
			{
				for (int k = 0; k < n_F + m_H; k++)
				{
					pKELi[i][j] += pVk[k+i*(n_F+m_H)] * pPEW[k][j];
				}
			}
		}


		// Matrix L
		double** pJ = myalloc2(m_H, n_F);
		jacobian(TapeID_H, m_H, n_F, pX, pJ);

		double** pL = myalloc2(m_H, m_H);
		for (int i = 0; i < m_H; i++)
		{
			for (int j = 0; j < m_H; j++)	
			{
				for (int k = 0; k < n_F; k++)
				{
					pL[i][j] += pJ[i][k] * pVk[nu[j]+j-1+(m_H+n_F)*k];
				}
			}
		}


		// Beobachterverstärkung berechnen
		mxArray *param2[2];
		param2[0] = mxCreateDoubleMatrix(n_F, m_H, mxREAL);	// k_1, ..., k_(m_H)
		param2[1] = mxCreateDoubleMatrix(m_H, m_H, mxREAL);	// L


		madMatrix2Vector(pKELi, mxGetPr(param2[0]), n_F, m_H);
		madMatrix2Vector(pL, mxGetPr(param2[1]), m_H, m_H);

		mexCallMATLAB(1, plhs, 2, param2, "mrdivide");



		mxDestroyArray(param1[0]);
		mxDestroyArray(param1[1]);
		mxDestroyArray(param2[0]);
		mxDestroyArray(param2[1]);
		mxDestroyArray(Vkpp[0]);

		myfree3(pQSi);
		myfree3(pQRT);
		myfree2(pQR);
		myfree2(pP);
		myfree2(pPEW);
		myfree2(pJ);
		myfree2(pL);
		myfree2(pKELi);
	}


    return;   
}



// Binomialkoeffizient
int binomialCoeff(unsigned int n, unsigned int k)
{
    int res = 1;
 
    if ( k > n - k )
        k = n - k;
 
    for (int i = 0; i < k; ++i)
    {
        res *= (n - i);
        res /= (i + 1);
    }
 
    return res;
}

