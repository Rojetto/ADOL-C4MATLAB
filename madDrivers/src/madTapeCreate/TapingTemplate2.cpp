#include <cstdlib>
#include <iostream>
#include <time.h>
#include "adolc/adouble.h"
#include "adolc/adutils.h"

using namespace std;

int main(int argc, char *argv[])
{
	const short int TapeNumber = //%TapeNumber%;
	double yNull;
    
	// =====================
	int n    = //%n%;
	int m    = //%m%;
	int keep = //%keep%;
    double x0//%x0%;
	// =====================\n');

	
	// =====================
	trace_on(TapeNumber, keep);

	adouble* x = new adouble[n];
	adouble* y = new adouble[m];
    for (int i = 0; i < n; i++) x[i] <<= x0[i];
    
	// Insertion of function to be taped
	for (int i = 0; i < m; i++) y[i] >>= yNull;

	trace_off(TapeNumber);
	// =====================

	
	delete [] x;
	delete [] y;

	int info[11];
	tapestats(TapeNumber, info);

	time_t t = time(NULL);

	cout << "============================================" << endl;
	cout << "Factory generated on " << __DATE__ << " " << __TIME__ << endl;
	#ifdef __GNUC__
	cout << "with Gnu-Compiler " << __GNUC__ << "." << __GNUC_MINOR__;
	#if (__GNUC__ > 2)
	cout << "." << __GNUC_PATCHLEVEL__;
	#endif
	cout << endl << endl;
	#endif
	cout << "Tape successfully written on " << ctime(&t);
	cout << "Number of independent variables: " << info[0] << endl;
	cout << "Number of dependent variables: " << info[1] << endl;
	cout << "Maximum number of live active variables: " << info[2] << endl;
	cout << "Size of value stack (number of overwrites): " << info[3] << endl;
	cout << "Buffer size (kB): " << 8*info[4]/1024 <<endl;
	cout << "Total number of operations: " << info[5] << endl;
	cout << "Tape saved under number " << TapeNumber << endl;
	cout << "============================================" << endl;

	return EXIT_SUCCESS;
}
