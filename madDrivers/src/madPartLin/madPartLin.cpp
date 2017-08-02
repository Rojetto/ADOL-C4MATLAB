// madPartLin.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
extern "C" {
	#include "matrixlib.h"
}

using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_X			 1
#define MEXAD_IN_v	         2
#define MEXAD_IN_akt		 3
#define MEXAD_IN_pass		 4

// Position und Bedeutung der Rückgabewerte der MEX-Funktion	(also *plhs[])
#define MEXAD_OUT_uEing         0
#define MEXAD_OUT_L				1
#define MEXAD_OUT_M				2
#define MEXAD_OUT_C				3

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

void matCreateSubsystem(double** subsystem, double** system, int dim_1, int dim_2, double* dof1, double* dof2)
{
	for (int i = 0; i < dim_1; i++)
	{
		for (int j = 0; j < dim_2; j++)
		{
			int k = dof1[i] - 1; int l = dof2[j] - 1;
			subsystem[i][j] = system[k][l];
		}
	}
}

void matMult3Sub(double** matrixAusgang, double** matrixA, double** matrixB,double** matrixC, double** matrixSub, int dim_akt, int dim_pass )
{
	double** pMZ1temp = myalloc2(dim_akt, dim_pass);
	double** pMZ2temp = myalloc2(dim_akt, dim_akt);

	for (int i = 0; i < dim_akt; i++)
	{
		for (int j = 0; j < dim_pass; j++)
		{
			for (int k = 0; k < dim_pass; k++)
			{
				pMZ1temp[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}

	for (int i = 0; i < dim_akt; i++)
	{
		for (int j = 0; j < dim_akt; j++)
		{
			for (int k = 0; k < dim_pass; k++)
			{
				pMZ2temp[i][j] = pMZ2temp[i][j] + pMZ1temp[i][k] * matrixC[k][j];
			}
		}
	}

	for (int i = 0; i < dim_akt; i++)
	{
		for (int j = 0; j < dim_akt; j++)
		{
			matrixAusgang[i][j] = matrixSub[i][j] - pMZ2temp[i][j];
		}
	}
	myfree2(pMZ1temp);
	myfree2(pMZ2temp);
}

void matMultiplikation(double** Ergebnis, double** matrixA, double** matrixB, int nM1Zeilen, int nM1Spalten, int nM2Spalten)
{
	for (int i = 0; i < nM1Zeilen; i++)
	{
		for (int j = 0; j < nM2Spalten; j++)
		{
			for (int k = 0; k < nM1Spalten; k++)
			{
				Ergebnis[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}
}

void matvecMultiplikation(double* Ergebnis, double** matrixA, double* vectorB, int nM1Zeilen, int nM1Spalten)
{
	for (int i = 0; i < nM1Zeilen; i++)
	{
		for (int k = 0; k < nM1Spalten; k++)
		{
			Ergebnis[i] += matrixA[i][k] * vectorB[k];
		}
	}
}

/* **********************************************************************************************
 * *****	Übergabeteil / Gateway-Routine													*****
 * *****	==============================													*****
 * *****																					*****
 * *****	Programmeinsprungpunkt															*****
 * *****																					*****
 * *****	Aufruf in MATLAB: L = madLagrangePartLin(TapeID, X, u, akt, pass)		    	*****
 * *****					  [L, M] =  madLagrangePartLin(TapeID, X, u, akt, pass)			*****
 * *****					  [L, M, C] =  madLagrangePartLin(TapeID, X, u, akt, pass)		*****
 * *****					  [L, M, C, VM] =  madLagrangePartLin(TapeID, X, u, akt, pass)	*****
 * *****																					*****
 * **********************************************************************************************
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])   //...prhs[]: Eingabewerte
{
	// Variablendefinitionen für die Verwendung
	// Informationen aus Tape
	int n;						// Anzahl der unabh. Variablen
	int m;						// Anzahl der abh. Variablen
	int dimq;					// Dimension von generalisierten Koordinaten q; dimq = n/2
	int dim_akt;				// Anzahl der aktiven Freiheitsgrade
	int dim_pass;				// Anzahl der passiven Freiheitsgrade

	// Eingabeparameter
	double* pX;					// Zeiger auf die Matrix für Taylor-Koeff.
								// der unabh. Variablen
	double* v;                  // Zeiger auf den Vektor der Trajektorien
	double* akt;				// Zeiger auf aktive Freiheitsgrade
	double* pass;				// Zeiger auf passive Freiheitsgrade

	MexADCTagType TapeID;		// Tape-Kennzeichner


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
	size_t TapeInfo[STAT_SIZE];


	// Initialisierung der MEX-Funktion
	if (!MexInitialized)
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);


	// Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 5, 5);					//Aus madHelpers...madDrivers/src
	madCheckNumOutputs(nlhs, 0, 1);					//Aus madHelpers...madDrivers/src


	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId")) return;
	TapeID = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]);

	//	Anzahl der "n" unabhängigen und "m" abhängigen Variablen des Tapes
	tapestats(TapeID, TapeInfo);
	n = TapeInfo[0];
	m = TapeInfo[1];

	dimq = n / 2;					// Dimension von generalisierten Koordinaten q; dimq = n/2 s.o.

	if (m != 1)
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),
			"The number of dependent variables must be 1!");
	}
	// Auswahl der aktiven und passiven Koordinaten akt und pass
	akt = mxGetPr(prhs[MEXAD_IN_akt]);
	pass = mxGetPr(prhs[MEXAD_IN_pass]);

	// Dimension der aktiven und passiven Koord. abfragen und prüfen.
	dim_akt = mxGetN(prhs[MEXAD_IN_akt]);
	dim_pass = mxGetN(prhs[MEXAD_IN_pass]);
	if ((dim_akt + dim_pass != dimq))
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(InvalidInput), "Invalid Input of degrees of freedom");
	}

	// X einlesen
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	// Äußere Kräfte v einlesen
	if (!madCheckDim1c(prhs, MEXAD_IN_v, dim_akt, "v")) return;
	v = mxGetPr(prhs[MEXAD_IN_v]);

	double* uEing;											// Neuer Eingang
	double* pL;																// Zeiger auf die Rückgabematrix
	double* pC;																// Zeiger auf die Rückgabematrix
	double* pM;																// Zeiger auf die Rückgabematrix

	// Rückgabe
	plhs[MEXAD_OUT_uEing] = mxCreateDoubleMatrix(dimq, 1, mxREAL);			// Angabe des Rückgabetyps
	uEing = mxGetPr(plhs[MEXAD_OUT_uEing]);
	plhs[MEXAD_OUT_L] = mxCreateDoubleMatrix(n, 1, mxREAL);
	pL = mxGetPr(plhs[MEXAD_OUT_L]);
	plhs[MEXAD_OUT_M] = mxCreateDoubleMatrix(dimq, dimq, mxREAL);
	pM = mxGetPr(plhs[MEXAD_OUT_M]);
	plhs[MEXAD_OUT_C] = mxCreateDoubleMatrix(dimq, 1, mxREAL);
	pC = mxGetPr(plhs[MEXAD_OUT_C]);
	
	double** pHb	= myalloc2(dimq, dimq);									// Hesseteilmatrix B
	double** pHi	= myalloc2(dimq, dimq);									// inverse Hesseteilmatrix D
	double** pMh	= myalloc2(dimq, dimq);

	double** pM11	= myalloc2(dim_pass, dim_pass);							// M11
	double** pM11inv= myalloc2(dim_pass, dim_pass);							// Inverse von M11 
	double** pM12	= myalloc2(dim_pass, dim_akt);							// M12
	double** pM21	= myalloc2(dim_akt, dim_pass);							// M21
	double** pM22	= myalloc2(dim_akt, dim_akt);							// M22
	double** pM22s	= myalloc2(dim_akt, dim_akt);							// M22 strich (aus u = M22s*v2 + c2)

	double** pMZ1	= myalloc2(dim_akt, dim_pass);							// Matrix für Zwischenergebnisse	
	double*  pVZ1	= myalloc(dim_akt);										// Vektor für Zwischenergebnisse

	double* pC1		= myalloc(dim_pass);									// Vektor des passiven Systems
	double* pC2		= myalloc(dim_akt);										// Vektor des aktiven Systems
	double* pC2s	= myalloc(dim_akt);										// C2 strich (aus u = M22s*v2 + c2)

	double* uneu	= myalloc(dim_akt);										// Aus u = M22s*v2 + C2s

	double* pZw		= myalloc(dimq);										// Zwischenergebnis

	matError err;
	matMatrix *pHd			= matMatrixCreateZero(dimq, dimq, &err);		// Hesseteilmatrix D
	matMatrix *pM11invmat	= matMatrixCreateZero(dim_pass, dim_pass, &err);// Inverse von M11 als Matrixobjekt
	matMatrix *pM11mat		= matMatrixCreateZero(dim_pass, dim_pass, &err);// M11 als Matrixobjekt
	
																			
	//	Aufruf der Berechnungsprozedur
	double* pG = myalloc(n);												// Gradient von X
	gradient(TapeID, n, pX, pG);											// pG_i = dL/dq_i  €R^(10x1)

	double** pH = myalloc2(n, n);											// pH = (A B; C D)
	hessian(TapeID, n, pX, pH);

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j <= i - 1; j++)
		{
			pH[j][i] = pH[i][j];											// Befüllen der symmetrischen Elemente
		};
	};

	// Initialisierung von Zwischenwerten									
	for (int i = 0 ; i < dimq; i++)
	{
		pZw[i] = 0;
		pL[i+dimq] = 0;
	}

	// Einlesen der Hesseteilmatrix D 
	for (int i = 0; i < dimq; i++)											// Umwandlung in Matrix - Objekt
	{
		for (int j = 0; j < dimq; j++)
		{
			//pHd.setvalue(i,j,pH[i+dimq][j+dimq]);
			matMatrixValSet(pHd, i, j, pH[i+dimq][j+dimq]);
		}
	}
	
	// Einlesen der Hesseteilmatrix B
	for (int i = 0; i < dimq; i++)				
	{
		for (int j = 0; j < dimq; j++)
		{
			pHb[i][j] = pH[i][j+dimq];			// Für die Berechnung von C über pZw
		}
	}

	if(n==2)
	{
		for (int i = 0; i < dimq; i++)				
		{
			pL[i] = pX[i+dimq];
			pL[i+dimq] = (pow(pH[1][1],-1))*(pG[0] - pH[0][1]*pX[1] + v[0]);
		}

		pM[0]=pH[1][1];
		pC[0]=pG[0] - pH[0][1]*pX[1];
	}

	else
	{
		// Invertierung der Hesseteilmatrix D
		//pHd.invert();
		pHd = matMatrixCreateInv(pHd, &err);
	
		// Speichern der invertierten Matrix in pHi
		for (int i = 0; i < dimq; i++)										
		{
			for (int j = 0; j < dimq; j++)
			{
				//pHd.getvalue(i,j,pHi[i][j],xyz);
				pHi[i][j]=matMatrixValGet(pHd, i, j, &err);					// Rücktransformation in double
			}
		}

		// m_ij = (d^2/(dqp_2*dqp_j)) * L(q,qp)
		for (int i = 0; i < dimq; i++)				
		{
			for (int j = 0; j < dimq; j++)
			{
				pMh[i][j] = pH[i+dimq][j+dimq];								// M
			}
		}

		madMatrix2Vector(pMh, pM, dimq, dimq);								// Wandelt Matrix (pMh) in Vektor (pM) um (Schreibt alle Spalten untereinander)
	    
		// Berechnung der Matrizen M11, M12, M21, M22

		matCreateSubsystem(pM11, pMh, dim_pass, dim_pass, pass, pass);		// M11
		matCreateSubsystem(pM12, pMh, dim_pass, dim_akt,  pass, akt);		// M12
		matCreateSubsystem(pM21, pMh, dim_akt,  dim_pass, akt,  pass);		// M21
		matCreateSubsystem(pM22, pMh, dim_akt,  dim_akt,  akt,  akt);		// M22

		for (int i = 0; i < dim_pass; i++)									
		{
			for (int j = 0; j < dim_pass; j++)
			{
				matMatrixValSet(pM11mat, i, j, pM11[i][j]);					// Umwandeln von M11 in matMatrix - Objekt
			}
		}

		pM11invmat = matMatrixCreateInv(pM11mat, &err);						

		for (int i = 0; i < dim_pass; i++)
		{
			for (int j = 0; j < dim_pass; j++)
			{
				pM11inv[i][j] = matMatrixValGet(pM11invmat, i, j, &err);	// Erzeugen der Inversen pM11inv aus pM11invmat (Rücktransf. in double)
			}
		}

		// Berechnung von M22s
		matMult3Sub(pM22s, pM21, pM11inv, pM12, pM22, dim_akt, dim_pass);

		// Berechnung von (d^2)L/((dq)(dq_punkt))*(dq_punkt)		(Wird für C benötigt)
		for (int i = 0; i < dimq; i++)				
		{
			for (int j = 0; j < dimq; j++)
			{
				pZw[i] += pHb[j][i]*pX[j+dimq];
			}
		}
	
		// Berechnung von C(q,dq)	c_j = (d^2/(dq_i*dqp_j))*L(q,qp)*qp - pG
		for (int j = 0; j < dimq; j++)
		{
			pC[j] = pZw[j] - pG[j];
		}

		// Besetzung von C1 und C2
		for (int i = 0; i < dim_pass; i++)
		{
			int k = pass[i] - 1;
			pC1[i] = pC[k];
		}
		for (int i = 0; i < dim_akt; i++)
		{
			int k = akt[i] - 1;
			pC2[i] = pC[k];
		}
		
		// Berechnung von C2s
		//matMult3Sub(pC2s, pM21, pM11inv, pC1, pC2, dim_akt, dim_pass);
		matMultiplikation(pMZ1, pM21, pM11inv, dim_akt, dim_pass, dim_pass);
		matvecMultiplikation(pVZ1, pMZ1, pC1, dim_akt, dim_pass);
		for (int i = 0; i < dim_akt; i++)
		{
			pC2s[i] = pC2[i] - pVZ1[i];
		}

		// Berechnung von u_neu (aus uneu = M22s * v + C2s)
		matvecMultiplikation(uneu, pM22s, v, dim_akt, dim_akt);						// Zwischenergebnis M22s * v
		for (int i = 0; i < dim_akt; i++)
		{
			uneu[i] += pC2s[i];
		}
		
		// Auffüllen von uneu mit den passiven Eingängen (Nullen)
		for (int j = 0; j < dim_akt; j++)
		{
			int k = akt[j]-1;
			uEing[k] = uneu[j];
		}
	}

	//Freigabe des Speichers
	myfree(pG);
	myfree2(pH);
	myfree2(pHb);
	myfree2(pHi);
	myfree2(pMh);
	myfree(pZw);

	matMatrixDelete(&pHd);
	matMatrixDelete(&pM11invmat);
	matMatrixDelete(&pM11mat);

	myfree2(pM11);
	myfree2(pM11inv);
	myfree2(pM12);
	myfree2(pM21);
	myfree2(pM22);
	myfree2(pM22s);
	myfree2(pMZ1);

	myfree(pVZ1);
	myfree(pC1);
	myfree(pC2);
	myfree(pC2s);

	myfree(uneu);

    return;   
}
