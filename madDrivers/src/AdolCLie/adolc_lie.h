/**************************************************************************
 * File: adolc_lie.h
 *
 * AdolCLie - a library for computation of several kinds of Lie derivatives
 *
 * If you do compile this using C include adolc_lie_c.c in your project
 * or Makefile.
 *
 * If you do compile this using C++ include adolc_lie_c.c as well as
 * adolc_lie.cpp in your project or Makefile.
 *
 * You have to provide the path to adolc.h and adouble.h of the software
 * package ADOL-C in order to compile the files properly.
 *
 * You have to link this library against adolc.dll (under Windows) or
 * libadolc.so / libadolc.a (under Linux), respectively in order to
 * build a library or an excecutable.
 * 
 * Use these conditionals (on Windows only):
 * ADOLC_LIE_BUILD: Set this preprocessor definition if you build a 
 *                  library from the sources. Do not set this macro if you
 *                  include this header in sources that import functions from
 *                  the library.
 *
 * Authors: Siquian Wang, Klaus Röbenack, Jan Winkler
 ***************************************************************************/
#if !defined(ADOLC_LIE_TOOL_H)
#define ADOLC_LIE_TOOL_H


#ifdef _WIN32  // (auch für WIN64 definiert, ebenso in MINGW)
#ifdef ADOLC_LIE_BUILD
#define ADOLC_API __declspec(dllexport)
#else
#define ADOLC_API __declspec(dllimport)
#endif
#else // Linux
#define ADOLC_API
#endif


// C++ declarations available only when compiling with C++
#if defined(__cplusplus)
ADOLC_API int lie_scalar(short, short, short, double*, short, double*);
ADOLC_API int lie_scalar(short, short, short, short, double*, short, double**);
ADOLC_API int lie_gradient(short, short, short, double*, short, double**);
ADOLC_API int lie_gradient(short, short, short, short, double*, short, double***);
ADOLC_API int lie_covector(short, short, short, double*, short, double**);
ADOLC_API int lie_bracket(short, short, short, double*, short, double**);
#endif



// C-declarations
#if defined (__cplusplus)
extern "C" {
#endif

ADOLC_API void accodeout(int, int, int, double***, double***, double***);// Formel 3.28 aus dem Buch, Bezeichnungaenderung
ADOLC_API void acccov(int, int, double***, double**, double**);          // Gleichung (17) von dem Material "Ueberlegungen..."
ADOLC_API void accadj(int, int, double***, double***);                   // Gleichung (26) von dem Material "Ueberlegungen..."
ADOLC_API void accbrac(int, int, double***, double**, double**);         // Gleichungen (29+30) von dem Material "Ueberlegungen..."
 
ADOLC_API int lie_scalarc(short, short, short, double*, short, double*);
ADOLC_API int lie_scalarcv(short, short, short, short, double*, short, double**);
ADOLC_API int lie_gradientc(short, short, short, double*, short, double**);
ADOLC_API int lie_gradientcv(short, short, short, short, double*, short, double***);
ADOLC_API int lie_covectorv(short, short, short, double*, short, double**);
ADOLC_API int lie_bracketv(short, short, short, double*, short, double**);

#if defined (__cplusplus)
}
#endif

#endif

