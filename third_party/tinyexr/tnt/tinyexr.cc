#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

// We want to use libz
#define TINYEXR_USE_MINIZ 0
#define TINYEXR_IMPLEMENTATION
#include <zlib.h>
#include "tinyexr.h"
