/**
 * (c) 2010-2018 
 * Mirko Franke, Jan Winkler, Carsten Friede
 * Institute of Control Theory
 * Technische Universität Dresden
 * {Mirko.Franke, Jan.Winkler}@tu-dresden.de
 */

#include "mex.h"
#include "adolc/adolc.h"
#include "madHelpers.h"
#include <iostream>
#include <time.h>

#define MEXAD_IN_TAPE_ID          0


static bool MexInitialized = false;   
static MexADCTagType TapeID = 0;
static mxArray *persistent_array_ptr = NULL;


// cleanup routine
void cleanup(void) 
{
	mexPrintf("Tape # %d (%s) closed\n", TapeID, __FILE__);
	mxDestroyArray(persistent_array_ptr);
	MexInitialized = false;
}


// main mex routine
void mexFunction( int nlhs, mxArray *plhs[],  int nrhs, const mxArray *prhs[] )  
{ 
    // initialization of the mex file
 	if (!MexInitialized) 
		MexInitialized = madInitialize(__FILE__, &persistent_array_ptr, cleanup);

    // check number of input and return parameters
	madCheckNumInputs(nrhs, 1, 1);
	madCheckNumOutputs(nlhs, 0, 0);        
    
	// TapeID
	if (!CheckIfScalar(prhs, MEXAD_IN_TAPE_ID, "TapeId")) return; 
	TapeID = (MexADCTagType)mxGetScalar(prhs[MEXAD_IN_TAPE_ID]); 


	double yNull;

	// =====================
	int n    = //%n%;
	int m    = //%m%;
	int keep = //%keep%;
	// =====================\n');

	adouble* x = new adouble[n];
	adouble* y = new adouble[m];

	// =====================
	trace_on(TapeID, keep);

	for (int i = 0; i < n; i++) x[i] <<= i;

	// Insertion of function to be taped

	for (int i = 0; i < m; i++) y[i] >>= yNull;

	trace_off(0);
	// =====================

	delete[] x;
	delete[] y;

	time_t t = time(NULL);
	mexPrintf("\n=====================================================\n");
	mexPrintf("Factory generated on             %s %s\n", __DATE__, __TIME__);
	mexPrintf("Tape successfully written on %s", ctime(&t));
	mexPrintf("=====================================================\n");

	size_t stats[STAT_SIZE];
	tapestats(TapeID, (size_t *)&stats);
	mexPrintf("\n*** TAPE STATS (tape %d) **********\n", (int)TapeID);
	mexPrintf("Number of independents: %10u\n", stats[NUM_INDEPENDENTS]);
	mexPrintf("Number of dependents:   %10u\n", stats[NUM_DEPENDENTS]);
	mexPrintf("\n");
	mexPrintf("Max # of live adoubles: %10u\n", stats[NUM_MAX_LIVES]);
	mexPrintf("Taylor stack size:      %10u\n", stats[TAY_STACK_SIZE]);
	mexPrintf("\n");
	mexPrintf("Number of operations:   %10u\n", stats[NUM_OPERATIONS]);
	mexPrintf("Number of locations:    %10u\n", stats[NUM_LOCATIONS]);
	mexPrintf("Number of values:       %10u\n", stats[NUM_VALUES]);
	mexPrintf("Number of parameters:   %10u\n", stats[NUM_PARAM]);
	mexPrintf("\n");
	mexPrintf("Operation file written: %10u\n", stats[OP_FILE_ACCESS]);
	mexPrintf("Location file written:  %10u\n", stats[LOC_FILE_ACCESS]);
	mexPrintf("Value file written:     %10u\n", stats[VAL_FILE_ACCESS]);
	mexPrintf("**********************************\n\n");

    return;   
}


