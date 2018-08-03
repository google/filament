/* def.c - define format of libpng.def
 *
 * Last changed in libpng version 1.6.16 [December 22, 2014]
 * Copyright (c) 2011-2014 Glenn Randers-Pehrson
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

/* Write the export file header: */
PNG_DFN ";--------------------------------------------------------------"
PNG_DFN "; LIBPNG module definition file for OS/2"
PNG_DFN ";--------------------------------------------------------------"
PNG_DFN ""
PNG_DFN "; If you give the library an explicit name one or other files"
PNG_DFN "; may need modifying to support the new name on one or more"
PNG_DFN "; systems."
PNG_DFN "LIBRARY"
PNG_DFN "OS2 DESCRIPTION "PNG image compression library""
PNG_DFN "OS2 CODE PRELOAD MOVEABLE DISCARDABLE"
PNG_DFN ""
PNG_DFN "EXPORTS"
PNG_DFN ";Version 1.6.16"

#define PNG_EXPORTA(ordinal, type, name, args, attributes)\
        PNG_DFN "@" SYMBOL_PREFIX "@@" name "@"

#include "../png.h"
