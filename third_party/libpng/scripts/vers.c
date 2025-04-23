/* vers.c - define format of libpng.vers
 *
 * Copyright (c) 2011-2014 Glenn Randers-Pehrson
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#define PNG_EXPORTA(ordinal, type, name, args, attributes)\
        PNG_DFN " @" SYMBOL_PREFIX "@@" name "@;"

PNG_DFN "@" PNGLIB_LIBNAME "@ {global:"

#include "../png.h"

PNG_DFN "local: *; };"
