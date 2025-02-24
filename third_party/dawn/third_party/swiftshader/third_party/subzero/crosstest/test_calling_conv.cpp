//===- subzero/crosstest/test_calling_conv.cpp - Implementation for tests -===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the test functions used to check that Subzero
// generates code compatible with the calling convention used by
// llc. "Caller" functions test the handling of out-args, and "callee"
// functions test the handling of in-args.
//
//===----------------------------------------------------------------------===//

#include <cstring>

#include "test_calling_conv.h"
#include "xdefs.h"

#define CALL_AS_TYPE(Ty, Func) (reinterpret_cast<Ty *>(Func))

void caller_i(void) {
  int arg1 = 0x12345678;
  CALL_AS_TYPE(callee_i_Ty, Callee)(arg1);
}

void caller_vvvvv(void) {
  v4si32 arg1 = {0, 1, 2, 3};
  v4si32 arg2 = {4, 5, 6, 7};
  v4si32 arg3 = {8, 9, 10, 11};
  v4si32 arg4 = {12, 13, 14, 15};
  v4si32 arg5 = {16, 17, 18, 19};

  CALL_AS_TYPE(callee_vvvvv_Ty, Callee)(arg1, arg2, arg3, arg4, arg5);
}

void caller_vlvilvfvdviv(void) {
  v4f32 arg1 = {0, 1, 2, 3};
  int64 arg2 = 4;
  v4f32 arg3 = {6, 7, 8, 9};
  int arg4 = 10;
  int64 arg5 = 11;
  v4f32 arg6 = {12, 13, 14, 15};
  float arg7 = 16;
  v4f32 arg8 = {17, 18, 19, 20};
  double arg9 = 21;
  v4f32 arg10 = {22, 23, 24, 25};
  int arg11 = 26;
  v4f32 arg12 = {27, 28, 29, 30};

  CALL_AS_TYPE(callee_vlvilvfvdviv_Ty, Callee)
  (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12);
}

#define HANDLE_ARG(ARGNUM)                                                     \
  case ARGNUM:                                                                 \
    memcpy(&Buf[0], &arg##ARGNUM, sizeof(arg##ARGNUM));                        \
    break;

void __attribute__((noinline)) callee_i(int arg1) {
  switch (ArgNum) { HANDLE_ARG(1); }
}

void __attribute__((noinline))
callee_vvvvv(v4si32 arg1, v4si32 arg2, v4si32 arg3, v4si32 arg4, v4si32 arg5) {
  switch (ArgNum) {
    HANDLE_ARG(1);
    HANDLE_ARG(2);
    HANDLE_ARG(3);
    HANDLE_ARG(4);
    HANDLE_ARG(5);
  }
}

void __attribute__((noinline))
callee_vlvilvfvdviv(v4f32 arg1, int64 arg2, v4f32 arg3, int arg4, int64 arg5,
                    v4f32 arg6, float arg7, v4f32 arg8, double arg9,
                    v4f32 arg10, int arg11, v4f32 arg12) {
  switch (ArgNum) {
    HANDLE_ARG(1);
    HANDLE_ARG(3);
    HANDLE_ARG(6);
    HANDLE_ARG(8);
    HANDLE_ARG(10);
    HANDLE_ARG(12);
    HANDLE_ARG(2);
    HANDLE_ARG(4);
    HANDLE_ARG(5);
    HANDLE_ARG(7);
    HANDLE_ARG(9);
    HANDLE_ARG(11);
  }
}
