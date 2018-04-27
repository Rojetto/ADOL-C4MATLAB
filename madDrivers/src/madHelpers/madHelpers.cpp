#include "madHelpers.h"

bool madInitialize(const char* const FileName, 
				   mxArray** persistent_array_ptr, 
				   void (*ExitFcn)(void))
{
   mexPrintf("Initializing %s\n", FileName);
     
   // Persistenz sichern
   *persistent_array_ptr = mxCreateDoubleMatrix(1, 1, mxREAL);
   mexMakeArrayPersistent(*persistent_array_ptr);
   mexAtExit(ExitFcn);       
        
   // Flag setzen
   return true;
};


void madCheckNumInputs(int nrhs, int MinNum, int MaxNum)
{
    if (nrhs > MaxNum)
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(TooManyInputs), "Too many inputs"); 
    } 
	else if (nrhs < MinNum)
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(TooFewInputs), "Too few inputs"); 
    } 
};



void madCheckNumOutputs(int nlhs, int MinNum, int MaxNum)
{
    if (nlhs > MaxNum)
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(TooManyOutputs), "Too many outputs"); 
    } 
	else if (nlhs < MinNum)
    { 
	   mexErrMsgIdAndTxt(MEXADC_ErrId(TooFewOutputs), "Too few outputs"); 
    } 
};


bool CheckIfScalar(const mxArray *ptr[], int ind, const char* const info)
{
   if (mxGetNumberOfElements(ptr[ind]) != 1)
   {
	   mexErrMsgIdAndTxt(MEXADC_ErrId(FScalar), "Input argument %d (%s) must be a scalar!", ind, info);
	   return false;
   };
   return true;
};

bool madCheckDim1r(const mxArray *ptr[], int ind, int nc, const char* const info)
{
	mwSize NumDim  = mxGetNumberOfDimensions(ptr[ind]);
	mwSize NumRows = mxGetM(ptr[ind]);
	mwSize NumCols = mxGetN(ptr[ind]);
	if ( (NumDim != 2) || (NumRows != 1) || (NumCols != nc) )
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(WrongMatrixDim), "Input argument %d (%s) must be a row-vector with %d rows", ind, info, nc);
		return false;
	}
	return true;
};

bool madCheckDim1c(const mxArray *ptr[], int ind, int nr, const char* const info)
{
	mwSize NumDim  = mxGetNumberOfDimensions(ptr[ind]);
	mwSize NumRows = mxGetM(ptr[ind]);
	mwSize NumCols = mxGetN(ptr[ind]);
	if ( (NumDim != 2) || (NumRows != nr) || (NumCols != 1) )
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(WrongMatrixDim), "Input argument %d (%s) must be a row-vector with %d columns", ind, info, nr);
		return false;
	}
	return true;
};


bool madCheckDim2(const mxArray *ptr[], int ind, int nr, int nc, const char* const info)
{
	mwSize NumDim      = mxGetNumberOfDimensions(ptr[ind]);
    const mwSize *dims = mxGetDimensions(ptr[ind]);
	if ( (NumDim != 2) || (dims[0] != nr) || (dims[1] != nc) )
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(WrongMatrixDim), "Input argument %d (%s) must be a %d x %d matrix", ind, info, nr, nc);
		return false;
	}
	return true;
};

bool madCheckDim3(const mxArray *ptr[], int ind, int nr, int nc, int np, const char* const info)
{
	mwSize NumDim      = mxGetNumberOfDimensions(ptr[ind]);
    const mwSize *dims = mxGetDimensions(ptr[ind]);
	if ( (NumDim != 3) || (dims[0] != nr) || (dims[1] != nc) || (dims[2] != np) )
	{
		mexErrMsgIdAndTxt(MEXADC_ErrId(WrongMatrixDim), "Input argument %d (%s) must be a %d x %d x %d tensor", ind, info, nr, nc, np);
		return false;
	}
	return true;
};

void madMatrix2Vector(double** pM, double* pV, int nr, int nc)
{
	for (int i = 0; i < nc; i++)
	{
		for (int j = 0; j < nr; j++)
		{
			pV[i*nr + j] = pM[j][i];
		};
	};
};
	
void madVector2Matrix(double* pV, double** pM, int nr, int nc)
{
	for (int i = 0; i < nc; i++)
	{
		for (int j = 0; j < nr; j++)
		{
			pM[j][i] = pV[i*nr + j];
		};
	};
};





	