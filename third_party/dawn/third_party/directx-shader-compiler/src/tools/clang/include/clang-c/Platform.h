/*===-- clang-c/Platform.h - C Index platform decls   -------------*- C -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header provides platform specific macros (dllimport, deprecated, ...) *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/
#ifndef LLVM_CLANG_C_PLATFORM_H
#define LLVM_CLANG_C_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/* MSVC DLL import/export. */
#ifndef CINDEX_LINKAGE

#ifdef _MSC_VER
  #ifdef _CINDEX_LIB_
    #define CINDEX_LINKAGE __declspec(dllexport)
  #else
    #define CINDEX_LINKAGE __declspec(dllimport)
  #endif
#else
  #define CINDEX_LINKAGE
#endif

#endif

// HLSL Change starts.
// This includes the LIBCLANG_CC inclusion in this file to be explicit
// about the calling convenion in use.
#ifndef LIBCLANG_CC
#ifdef _MSC_VER
// DLL APIs should use the standard call convention like other WINAPI APIs.
#define LIBCLANG_CC __stdcall
#else
#define LIBCLANG_CC
#endif
#endif
// HLSL Change ends.

#ifdef __GNUC__
  #define CINDEX_DEPRECATED __attribute__((deprecated))
#else
  #ifdef _MSC_VER
    #define CINDEX_DEPRECATED __declspec(deprecated)
  #else
    #define CINDEX_DEPRECATED
  #endif
#endif

#ifdef __cplusplus
}
#endif
#endif
