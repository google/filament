/*===-- clang-c/CXString.h - C Index strings  --------------------*- C -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header provides the interface to C Index strings.                     *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/
#ifndef LLVM_CLANG_C_CXSTRING_H
#define LLVM_CLANG_C_CXSTRING_H

#include "clang-c/Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup CINDEX_STRING String manipulation routines
 * \ingroup CINDEX
 *
 * @{
 */

/**
 * \brief A character string.
 *
 * The \c CXString type is used to return strings from the interface when
 * the ownership of that string might differ from one call to the next.
 * Use \c clang_getCString() to retrieve the string data and, once finished
 * with the string data, call \c clang_disposeString() to free the string.
 */
typedef struct {
  const void *data;
  unsigned private_flags;
} CXString;

/**
 * \brief Retrieve the character data associated with the given string.
 */
CINDEX_LINKAGE const char * LIBCLANG_CC clang_getCString(CXString string);

/**
 * \brief Free the given string.
 */
CINDEX_LINKAGE void LIBCLANG_CC clang_disposeString(CXString string);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif

