#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

// We want multi-threading
#define TINYEXR_USE_THREAD 1

// We want to use libz
#define TINYEXR_USE_MINIZ 0
#define TINYEXR_IMPLEMENTATION
#include <zlib.h>
#include "tinyexr.h"
