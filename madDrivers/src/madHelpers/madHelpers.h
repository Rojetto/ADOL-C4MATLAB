#ifndef MADHELPERS_H
#define MADHELPERS_H

#include "mex.h"

// Datentyp für das Tag des Tapes
typedef unsigned short MexADCTagType;

// Für Mex-File Fehlerreport
#define MEXADC_ErrId(T)          "mexAD:T"

bool madInitialize(const char* const FileName, mxArray** persistent_array_ptr, 
				   void (*ExitFcn)(void));

void madCheckNumInputs(int nrhs, int MinNum, int MaxNum);
void madCheckNumOutputs(int nlhs, int MinNum, int MaxNum);

bool CheckIfScalar(const mxArray *ptr[], int ind, const char* const info);
bool madCheckDim1r(const mxArray *ptr[], int ind, int nr, const char* const info);
bool madCheckDim1c(const mxArray *ptr[], int ind, int nc, const char* const info);
bool madCheckDim2(const mxArray *ptr[], int ind, int nr, int nc, const char* const info);
bool madCheckDim3(const mxArray *ptr[], int ind, int nr, int nc, int np, const char* const info);

void madMatrix2Vector(double** pM, double* pV, int nr, int nc);
void madVector2Matrix(double* pV, double** pM, int nr, int nc);


#endif