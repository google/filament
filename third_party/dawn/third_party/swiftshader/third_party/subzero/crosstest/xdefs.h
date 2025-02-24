//===- subzero/crosstest/xdefs.h - Definitions for the crosstests. --------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Defines the int64 and uint64 types to avoid link-time errors when compiling
// the crosstests in LP64.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_CROSSTEST_XDEFS_H_
#define SUBZERO_CROSSTEST_XDEFS_H_

typedef unsigned int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned int SizeT;

#endif // SUBZERO_CROSSTEST_XDEFS_H_
