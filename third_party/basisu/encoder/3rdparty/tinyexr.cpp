#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable:4530) //  warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#endif

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
