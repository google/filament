; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=_main  -o %t2 %t1
; RUN: llvm-nm %t2 | FileCheck %s 

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

target triple = "x86_64-apple-macosx10.9"

declare double @pow(double, double)

define double @main(double %x) {
; We check that LTO will be aware of target triple and apply pow to __exp10 transformation.
; CHECK: U ___exp10
  %retval = call double @pow(double 10.0, double %x)
  ret double %retval
}
