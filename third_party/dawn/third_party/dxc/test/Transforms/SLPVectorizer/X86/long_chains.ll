; RUN: opt < %s -basicaa -slp-vectorizer -dce -S -mtriple=x86_64-apple-macosx10.8.0 -mcpu=corei7-avx | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.8.0"

; At this point we can't vectorize only parts of the tree.

; CHECK: test
; CHECK: insertelement <2 x i8>
; CHECK: insertelement <2 x i8>
; CHECK: sitofp <2 x i8>
; CHECK: fmul <2 x double>
; CHECK: ret
define i32 @test(double* nocapture %A, i8* nocapture %B) {
entry:
  %0 = load i8, i8* %B, align 1
  %arrayidx1 = getelementptr inbounds i8, i8* %B, i64 1
  %1 = load i8, i8* %arrayidx1, align 1
  %add = add i8 %0, 3
  %add4 = add i8 %1, 3
  %conv6 = sitofp i8 %add to double
  %conv7 = sitofp i8 %add4 to double 
  %mul = fmul double %conv6, %conv6
  %add8 = fadd double %mul, 1.000000e+00
  %mul9 = fmul double %conv7, %conv7
  %add10 = fadd double %mul9, 1.000000e+00
  %mul11 = fmul double %add8, %add8
  %add12 = fadd double %mul11, 1.000000e+00
  %mul13 = fmul double %add10, %add10
  %add14 = fadd double %mul13, 1.000000e+00
  %mul15 = fmul double %add12, %add12
  %add16 = fadd double %mul15, 1.000000e+00
  %mul17 = fmul double %add14, %add14
  %add18 = fadd double %mul17, 1.000000e+00
  %mul19 = fmul double %add16, %add16
  %add20 = fadd double %mul19, 1.000000e+00
  %mul21 = fmul double %add18, %add18
  %add22 = fadd double %mul21, 1.000000e+00
  %mul23 = fmul double %add20, %add20
  %add24 = fadd double %mul23, 1.000000e+00
  %mul25 = fmul double %add22, %add22
  %add26 = fadd double %mul25, 1.000000e+00
  store double %add24, double* %A, align 8
  %arrayidx28 = getelementptr inbounds double, double* %A, i64 1
  store double %add26, double* %arrayidx28, align 8
  ret i32 undef
}
