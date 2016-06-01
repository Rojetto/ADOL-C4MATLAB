// madFeedbackLin.cpp


// Benötigte Header und Namensräume
#include "mex.h"
#include "adolc\adolc.h"
#include "madHelpers.h"
#include "adolc\lie\drivers.h"


using namespace std;


// Position und Bedeutung der Eingabeparameter der Mex-Funktion (also *prhs[])
#define MEXAD_IN_TAPE_F     0
#define MEXAD_IN_TAPE_FG	1
#define MEXAD_IN_TAPE_H     2
#define MEXAD_IN_X			3
#define MEXAD_IN_K          4
#define MEXAD_IN_YREF		5


// Position und Bedeutung der Rückgabewerte der MEX-Funktion (also *plhs[])
#define MEXAD_OUT_U         0
#define MEXAD_OUT_DELTA		1


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



/* ******************************************************************************************************
 * *****	Übergabeteil / Gateway-Routine															*****
 * *****	==============================															*****
 * *****																							*****
 * *****	Programmeinsprungpunkt																	*****
 * *****																							*****
 * *****	Aufruf in MATLAB:         u = madFeedbackLin( TapeIdF, TapeIdFG, TapeIdH, X, k, yref );	*****
 * *****                              U = madFeedbackLin( TapeIdF, TapeIdFG, TapeIdH, X, K, Yref );	*****
 * *****                      [U,Delta] = madFeedbackLin( TapeIdF, TapeIdFG, TapeIdH, X, K, Yref );	*****
 * *****																						    *****
 * ******************************************************************************************************
 */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	// Variablendefinitionen für die Verwendung 
	double* ptrOutput;					// Zeiger auf die Rückgabematrix
	double* pX;							// Zeiger auf Werte der unabh. Variablen
	double* pK;							// Zeiger auf die Verstärkungsmatrix
	double* pYref;						// Zeiger auf die Referenzausgangsmatrix

	MexADCTagType TapeID_F;				// Tape-Kennzeichner f
	MexADCTagType TapeID_FG;			//		 -//-		 f+g
	MexADCTagType TapeID_H;				//		 -//-		 h
	int m_F;							// Anzahl der abh. Variablen von f
	int n_F;							// Anzahl der unabh. Variablen von f
	int m_FG;							// Anzahl der abh. Variablen von f+g
	int n_FG;							// Anzahl der unabh. Variablen von f+g
	int m_H;							// Anzahl der abh. Variablen
	int n_H;							// Anzahl der unabh. Variablen


	//	Array mit Informationen zum Tape (siehe AdolC\taping.c, Funktion tapestats())
	size_t TapeInfo_F[STAT_SIZE];

	size_t TapeInfo_FG[STAT_SIZE];

	size_t TapeInfo_H[STAT_SIZE];


	// Initialisierung der MEX-Funktion
	if (!MexInitialized)
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);


	// Prüfen der Anzahl der Eingabe- und Rückgabeargumente
	madCheckNumInputs(nrhs, 6, 6);
	madCheckNumOutputs(nlhs, 0, 2);


	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_F, "TapeId_F")) return;
	TapeID_F = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_F]);

	//	Anzahl der "n_F" unabhängigen und "m_F" abhängigen Variablen des Tapes_F
	tapestats(TapeID_F, TapeInfo_F);
	n_F = TapeInfo_F[NUM_INDEPENDENTS];
	m_F = TapeInfo_F[NUM_DEPENDENTS];

	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_FG, "TapeId_FG")) return;
	TapeID_FG = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_FG]);

	//	Anzahl der "n_FG" unabhängigen und "m_FG" abhängigen Variablen des Tapes_FG
	tapestats(TapeID_FG, TapeInfo_FG);
	n_FG = TapeInfo_FG[NUM_INDEPENDENTS];
	m_FG = TapeInfo_FG[NUM_DEPENDENTS];

	// Tape_ID ermitteln und zugehörige Informationen des Tapes in Array TapeInfo speichern
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_H, "TapeId_H")) return;
	TapeID_H = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_H]);

	//	Anzahl der "n_H" unabhängigen und "m_H" abhängigen Variablen des Tapes_H
	tapestats(TapeID_H, TapeInfo_H);
	n_H = TapeInfo_H[NUM_INDEPENDENTS];
	m_H = TapeInfo_H[NUM_DEPENDENTS];

	if ((n_F != m_F) || (n_F != n_H))
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),
			"The number of dependent and independent variables of f must be identical to the number of independent variables of h!");
	}
	if ((n_FG != m_FG) || (n_FG != n_H))
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(TapeMismatch),
			"The number of dependent and independent variables of f+g must be identical to the number of independent variables of h!");
	}

	// X
	if (!madCheckDim1c(prhs, MEXAD_IN_X, n_F, "X")) return;
	pX = mxGetPr(prhs[MEXAD_IN_X]);

	// k bzw. K
	if (m_H == 1)	// SISO
	{
		//if (!madCheckDim1c(prhs, MEXAD_IN_K, n_F, "k")) return;
		if (!1==mxGetM(prhs[MEXAD_IN_K])) return;
		pK = mxGetPr(prhs[MEXAD_IN_K]);
	}
	else			// MIMO
	{
		//if (!madCheckDim2(prhs, MEXAD_IN_K, n_F, m_H, "K")) return;
		//pK = mxGetPr(prhs[MEXAD_IN_K]);
	}

	// yref bzw. Yref
	if (m_H == 1)	// SISO
	{
		//if (!madCheckDim1c(prhs, MEXAD_IN_YREF, n_F, "yref")) return;
		if (!1 == mxGetM(prhs[MEXAD_IN_YREF])) return;
		pYref = mxGetPr(prhs[MEXAD_IN_YREF]);
	}
	else			// MIMO
	{
		//if (!madCheckDim2(prhs, MEXAD_IN_Yref, n_F, m_H, "Yref")) return;
		//pK = mxGetPr(prhs[MEXAD_IN_K]);
	}



	// SISO
	if (m_H == 1)
	{
		int r = mxGetN(prhs[MEXAD_IN_K]) - 1;	// relativer Grad

		double* pdy1 = myalloc(r + 1);
		double* pdy2 = myalloc(r + 1);

		double u = 0.;

		// Zeiger für Rückgabe
		plhs[MEXAD_OUT_U] = mxCreateDoubleMatrix(1, 1, mxREAL);
		ptrOutput = mxGetPr(plhs[MEXAD_OUT_U]);


		// Berechnungsprozedur 
		// Lie-Ableitungen von h entlang f
		lie_scalar(TapeID_F, TapeID_H, n_F, pX, r,pdy1);
		// Lie-Ableitungen von h entlang f+g
		lie_scalar(TapeID_FG, TapeID_H, n_F, pX, r, pdy2);

		for (int i = 0; i <= r; i++)
			u += pK[i]*(pYref[i]-pdy1[i]);

		// Rückgabe
		ptrOutput[0] = u / (pdy2[r] - pdy1[r]);

		myfree(pdy1);
		myfree(pdy2);
	}


	// MIMO
	else
	{

	}

	return;
}