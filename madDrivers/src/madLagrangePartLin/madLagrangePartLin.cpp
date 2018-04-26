// madLagrangePartLin.cpp


// Benötigte Header
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
extern "C" {
	#include "matrixlib.h"
}


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F      0
#define MEXAD_IN_X			 1
#define MEXAD_IN_v	         2
#define MEXAD_IN_akt		 3
#define MEXAD_IN_pass		 4

// Position und Bedeutung der Rückgabewerte der MEX-Funktion	(also *plhs[])
#define MEXAD_OUT_U          0
#define MEXAD_OUT_L			 1
#define MEXAD_OUT_M			 2
#define MEXAD_OUT_C			 3

// wird nach dem 1. Aufruf auf true gesetzt
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


void matCreateSubsystem(double** subsystem, double** system, int dim_1, int dim_2, double* dof1, double* dof2)
{
	for (int i = 0; i < dim_1; i++)
	{
		for (int j = 0; j < dim_2; j++)
		{
			int k = (int)(dof1[i]) - 1; int l = (int)(dof2[j]) - 1;
			subsystem[i][j] = system[k][l];
		}
	}
}


void matMultiplikation(double** result, double** matrixA, double** matrixB, int rowsA, int columnsA, int columnsB)
{
	for (int i = 0; i < rowsA; i++)
	{
		for (int j = 0; j < columnsB; j++)
		{
			for (int k = 0; k < columnsA; k++)
			{
				result[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}
}


void matVecMultiplikation(double* result, double** matrix, double* vector, int rows, int columns)
{
	for (int i = 0; i < rows; i++)
	{
		for (int k = 0; k < columns; k++)
		{
			result[i] += matrix[i][k] * vector[k];
		}
	}
}


int my_hessian(short tag,
	int n,
	double* argument,
	double** hess)
{
	int rc = 3;
	int i, j;
	double *v = myalloc1(n);
	double *w = myalloc1(n);
	for (i = n / 2; i<n; i++) v[i] = 0;
	for (i = n / 2; i<n; i++) {
		v[i] = 1;
		MINDEC(rc, hess_vec(tag, n, argument, v, w));
		if (rc < 0) {
			free((char *)v);
			free((char *)w);
			return rc;
		}
		for (j = 0; j <= i; j++)
			hess[j][i] = w[j];
		v[i] = 0;
	}

	free((char *)v);
	free((char *)w);
	return rc;
	/* Note that only the lower right triangle of hess is filled */
}


/* ***********************************************************************************
 * *****	Übergabeteil / Gateway-Routine										 *****
 * *****	==============================										 *****
 * *****																		 *****
 * *****	Programmeinsprungpunkt												 *****
 * *****																		 *****
 * *****	Aufruf in MATLAB:  U = madLagrangePartLin(TapeID, X, u, akt, pass)   *****
 * *****																		 *****
 * ***********************************************************************************
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])   //...prhs[]: Eingabewerte
{
	// Variablendefinitionen für die Verwendung 
	// Ausgabeparameter
	double* pU;					// Zeiger auf Rückgabewert (Eingang u)

	// Eingabeparameter
	double* pX;					// Zeiger auf die Matrix für Taylor-Koeff.
								// der unabh. Variablen
	double* v;                  // Zeiger auf den Vektor der Trajektorien
	double* akt;				// Zeiger auf aktive Freiheitsgrade
	double* pass;				// Zeiger auf passive Freiheitsgrade

	MexADCTagType TapeID;		// Tape-Kennzeichner

	// Informationen aus Tape
	int n;						// Anzahl der unabh. Variablen
	int m;						// Anzahl der abh. Variablen
	int dimq;					// Dimension von generalisierten Koordinaten q; dimq = n/2

	int dim_akt;				// Anzahl der aktiven Freiheitsgrade
	int dim_pass;				// Anzahl der aktiven Freiheitsgrade

	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
	size_t TapeInfo[STAT_SIZE];


	// Initialisierung der MEX-Funktion
	if (!MexInitialized)
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);


	// Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 5, 5);					//Aus madHelpers...madDrivers/src
	madCheckNumOutputs(nlhs, 1, 1);					//Aus madHelpers...madDrivers/src


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
		mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch), "The number of dependent variables must be 1!");
	}
	// Auswahl der aktiven und passiven Koordinaten akt und pass
	akt = mxGetPr(prhs[MEXAD_IN_akt]);
	pass = mxGetPr(prhs[MEXAD_IN_pass]);

	// Dimension der aktiven und passiven Koord. abfragen und prüfen.
	dim_akt = mxGetN(prhs[MEXAD_IN_akt]);
	dim_pass = mxGetN(prhs[MEXAD_IN_pass]);
	if ((dim_akt + dim_pass != dimq))
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(InvalidInput), "Invalid input of degrees of freedom");
	}

	// X einlesen
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	// Äußere Kräfte v einlesen
	if (!madCheckDim1c(prhs, MEXAD_IN_v, dim_akt, "v")) return;
	v = mxGetPr(prhs[MEXAD_IN_v]);

	// Rückgabe
	plhs[MEXAD_OUT_U] = mxCreateDoubleMatrix(dimq, 1, mxREAL);					// Angabe des Rückgabetyps
	pU = mxGetPr(plhs[MEXAD_OUT_U]);

	
	//	Aufruf der Berechnungsprozedur
	double* pC = myalloc(dimq);

	double* pG = myalloc(n);												// Gradient von X
	gradient(TapeID, n, pX, pG);											// pG_i = dL/dq_i  €R^(10x1)

	double** pH = myalloc2(n, n);											// pH = (0 B; 0 D)
	my_hessian(TapeID, n, pX, pH);

	double** pHb	= myalloc2(dimq, dimq);									// Hesseteilmatrix B
	double** pHi	= myalloc2(dimq, dimq);									// inverse Hesseteilmatrix D
	double** pMh	= myalloc2(dimq, dimq);

	double** pM11	= myalloc2(dim_akt, dim_akt);							// M11
	double** pM12	= myalloc2(dim_akt, dim_pass);							// M12
	double** pM21	= myalloc2(dim_pass, dim_akt);							// M21
	double** pM22	= myalloc2(dim_pass, dim_pass);							// M22
	double** pM22inv = myalloc2(dim_pass, dim_pass);						// Inverse von M22
	double** pM11s	= myalloc2(dim_akt, dim_akt);							// M11 strich (aus u = M11s*v + C1s)

	double** pMZ1	= myalloc2(dim_akt, dim_pass);							// Matrix für Zwischenergebnisse	
	double*  pVZ1	= myalloc(dim_akt);										// Vektor für Zwischenergebnisse

	double* pC2		= myalloc(dim_pass);									// Vektor des passiven Systems
	double* pC1s	= myalloc(dim_akt);										// C1 strich (aus u = M11s*v + C1s)

	double* uneu	= myalloc(dim_akt);										// Aus u = M22s*v2 + C2s

	double* pZw		= myalloc(dimq);										// Zwischenergebnis

	matError err;
	matMatrix *pHd			= matMatrixCreateZero(dimq, dimq, &err);		// Hesseteilmatrix D
	matMatrix *pM22invmat	= matMatrixCreateZero(dim_pass, dim_pass, &err);// Inverse von M11 als Matrixobjekt
	matMatrix *pM22mat		= matMatrixCreateZero(dim_pass, dim_pass, &err);// M11 als Matrixobjekt

	// Initialisierung von Zwischenwerten									
	for (int i = 0 ; i < dimq; i++)
	{
		pZw[i] = 0;
	}

	// Einlesen der Hesseteilmatrix D 
	for (int i = 0; i < dimq; i++)											// Umwandlung in Matrix - Objekt
	{
		for (int j = 0; j < dimq; j++)
		{
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

	// TODO!
	//if(n==2)
	//{
	//	pC[0]=pG[0] - pH[0][1]*pX[1];
	//}

	//else
	//{
		// Invertierung der Hesseteilmatrix D
		pHd = matMatrixCreateInv(pHd, &err);
	
		// Speichern der invertierten Matrix in pHi
		for (int i = 0; i < dimq; i++)										
		{
			for (int j = 0; j < dimq; j++)
			{
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
	    

		// Berechnung der Matrizen M11, M12, M21, M22
		matCreateSubsystem(pM11, pMh, dim_akt,  dim_akt,  akt,  akt);		// M11
		matCreateSubsystem(pM12, pMh, dim_akt,  dim_pass, akt,  pass);		// M12
		matCreateSubsystem(pM21, pMh, dim_pass, dim_akt,  pass, akt);		// M21
		matCreateSubsystem(pM22, pMh, dim_pass, dim_pass, pass, pass);		// M22

		for (int i = 0; i < dim_pass; i++)									
		{
			for (int j = 0; j < dim_pass; j++)
			{
				matMatrixValSet(pM22mat, i, j, pM22[i][j]);					// Umwandeln von M22 in matMatrix - Objekt
			}
		}

		pM22invmat = matMatrixCreateInv(pM22mat, &err);						

		for (int i = 0; i < dim_pass; i++)
		{
			for (int j = 0; j < dim_pass; j++)
			{
				pM22inv[i][j] = matMatrixValGet(pM22invmat, i, j, &err);	// Erzeugen der Inversen pM22inv aus pM22invmat (Rücktransf. in double)
			}
		}


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
			int k = (int)(pass[i]) - 1;
			pC2[i] = pC[k];
		}
		
		// Berechnung von C1s
		matMultiplikation(pMZ1, pM12, pM22inv, dim_akt, dim_pass, dim_pass);
		matVecMultiplikation(pVZ1, pMZ1, pC2, dim_akt, dim_pass);
		for (int i = 0; i < dim_akt; i++)
		{
			int k = (int)(akt[i]) - 1;
			pC1s[i] = - pVZ1[i] + pC[k];
		}

		// Berechnung von M22s
		matMultiplikation(pM11s, pMZ1, pM21, dim_akt, dim_pass, dim_akt);
		for (int i = 0; i < dim_akt; i++)
		{
			for (int j = 0; j < dim_akt; j++)
			{
				pM11s[i][j] = pM11[i][j] - pM11s[i][j];
			}
		}

		// Berechnung von u_neu (aus uneu = M11s * v + C1s)
		matVecMultiplikation(uneu, pM11s, v, dim_akt, dim_akt);						// Zwischenergebnis M22s * v
		for (int i = 0; i < dim_akt; i++)
		{
			uneu[i] += pC1s[i];
		}
		
		// Auffüllen von uneu mit den passiven Eingängen (Nullen)
		for (int j = 0; j < dim_akt; j++)
		{
			int k = (int)(akt[j])-1;
			pU[k] = uneu[j];
		}

	//}

	//Freigabe des Speichers
	myfree(pG);
	myfree2(pH);
	myfree2(pHb);
	myfree2(pHi);
	myfree2(pMh);
	myfree(pZw);

	matMatrixDelete(&pHd);
	matMatrixDelete(&pM22invmat);
	matMatrixDelete(&pM22mat);

	myfree2(pM11);
	myfree2(pM11s);
	myfree2(pM12);
	myfree2(pM21);
	myfree2(pM22);
	myfree2(pM22inv);
	myfree2(pMZ1);

	myfree(pVZ1);
	myfree(pC1s);
	myfree(pC2);

	myfree(pC);

	myfree(uneu);
	//myfree(uEing);

    return;   
}