#ifdef _WIN32
#define DLLEXPORT __ddeclspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <iostream>

/* Some library functions that can be "extern"ed form user code.
 *
 * */

extern "C" DLLEXPORT double putchard(double X);
extern "C" DLLEXPORT double printd(double X);
