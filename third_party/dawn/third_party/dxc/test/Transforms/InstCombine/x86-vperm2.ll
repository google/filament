; RUN: opt < %s -instcombine -S | FileCheck %s

; This should never happen, but make sure we don't crash handling a non-constant immediate byte.

define <4 x double> @perm2pd_non_const_imm(<4 x double> %a0, <4 x double> %a1, i8 %b) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 %b) 
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_non_const_imm
; CHECK-NEXT:  call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 %b)
; CHECK-NEXT:  ret <4 x double>
}


; In the following 4 tests, both zero mask bits of the immediate are set.

define <4 x double> @perm2pd_0x88(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 136) 
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x88
; CHECK-NEXT:  ret <4 x double> zeroinitializer
}

define <8 x float> @perm2ps_0x88(<8 x float> %a0, <8 x float> %a1) {
  %res = call <8 x float> @llvm.x86.avx.vperm2f128.ps.256(<8 x float> %a0, <8 x float> %a1, i8 136) 
  ret <8 x float> %res

; CHECK-LABEL: @perm2ps_0x88
; CHECK-NEXT:  ret <8 x float> zeroinitializer
}

define <8 x i32> @perm2si_0x88(<8 x i32> %a0, <8 x i32> %a1) {
  %res = call <8 x i32> @llvm.x86.avx.vperm2f128.si.256(<8 x i32> %a0, <8 x i32> %a1, i8 136) 
  ret <8 x i32> %res

; CHECK-LABEL: @perm2si_0x88
; CHECK-NEXT:  ret <8 x i32> zeroinitializer
}

define <4 x i64> @perm2i_0x88(<4 x i64> %a0, <4 x i64> %a1) {
  %res = call <4 x i64> @llvm.x86.avx2.vperm2i128(<4 x i64> %a0, <4 x i64> %a1, i8 136) 
  ret <4 x i64> %res

; CHECK-LABEL: @perm2i_0x88
; CHECK-NEXT:  ret <4 x i64> zeroinitializer
}


; The other control bits are ignored when zero mask bits of the immediate are set.

define <4 x double> @perm2pd_0xff(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 255) 
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0xff
; CHECK-NEXT:  ret <4 x double> zeroinitializer
}


; The following 16 tests are simple shuffles, except for 2 cases where we can just return one of the
; source vectors. Verify that we generate the right shuffle masks and undef source operand where possible..

define <4 x double> @perm2pd_0x00(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 0)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x00
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x01(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 1)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x01
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> undef, <4 x i32> <i32 2, i32 3, i32 0, i32 1>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x02(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 2)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x02
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> %a0, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x03(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 3)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x03
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> %a0, <4 x i32> <i32 2, i32 3, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x10(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 16)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x10
; CHECK-NEXT:  ret <4 x double> %a0
}

define <4 x double> @perm2pd_0x11(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 17)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x11
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> undef, <4 x i32> <i32 2, i32 3, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x12(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 18)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x12
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> %a0, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x13(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 19)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x13
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> %a0, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x20(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 32)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x20
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> %a1, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x21(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 33)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x21
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> %a1, <4 x i32> <i32 2, i32 3, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x22(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 34)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x22
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x23(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 35)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x23
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> undef, <4 x i32> <i32 2, i32 3, i32 0, i32 1>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x30(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 48)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x30
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> %a1, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x31(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 49)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x31
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a0, <4 x double> %a1, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
; CHECK-NEXT:  ret <4 x double> %1
}

define <4 x double> @perm2pd_0x32(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 50)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x32
; CHECK-NEXT:  ret <4 x double> %a1
}

define <4 x double> @perm2pd_0x33(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 51)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x33
; CHECK-NEXT:  %1 = shufflevector <4 x double> %a1, <4 x double> undef, <4 x i32> <i32 2, i32 3, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x double> %1
}

; Confirm that a mask for 32-bit elements is also correct.

define <8 x float> @perm2ps_0x31(<8 x float> %a0, <8 x float> %a1) {
  %res = call <8 x float> @llvm.x86.avx.vperm2f128.ps.256(<8 x float> %a0, <8 x float> %a1, i8 49)
  ret <8 x float> %res

; CHECK-LABEL: @perm2ps_0x31
; CHECK-NEXT:  %1 = shufflevector <8 x float> %a0, <8 x float> %a1, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:  ret <8 x float> %1
}


; Confirm that the AVX2 version works the same.

define <4 x i64> @perm2i_0x33(<4 x i64> %a0, <4 x i64> %a1) {
  %res = call <4 x i64> @llvm.x86.avx2.vperm2i128(<4 x i64> %a0, <4 x i64> %a1, i8 51)
  ret <4 x i64> %res

; CHECK-LABEL: @perm2i_0x33
; CHECK-NEXT:  %1 = shufflevector <4 x i64> %a1, <4 x i64> undef, <4 x i32> <i32 2, i32 3, i32 2, i32 3>
; CHECK-NEXT:  ret <4 x i64> %1
}


; Confirm that when a single zero mask bit is set, we replace a source vector with zeros.

define <4 x double> @perm2pd_0x81(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 129)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x81
; CHECK-NEXT:  shufflevector <4 x double> %a0, <4 x double> <double 0.0{{.*}}<4 x i32> <i32 2, i32 3, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double>
}

define <4 x double> @perm2pd_0x83(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 131)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x83
; CHECK-NEXT:  shufflevector <4 x double> %a1, <4 x double> <double 0.0{{.*}}, <4 x i32> <i32 2, i32 3, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double>
}

define <4 x double> @perm2pd_0x28(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 40)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x28
; CHECK-NEXT:  shufflevector <4 x double> <double 0.0{{.*}}, <4 x double> %a1, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double>
}

define <4 x double> @perm2pd_0x08(<4 x double> %a0, <4 x double> %a1) {
  %res = call <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double> %a0, <4 x double> %a1, i8 8)
  ret <4 x double> %res

; CHECK-LABEL: @perm2pd_0x08
; CHECK-NEXT:  shufflevector <4 x double> <double 0.0{{.*}}, <4 x double> %a0, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x double>
}

; Check one more with the AVX2 version.

define <4 x i64> @perm2i_0x28(<4 x i64> %a0, <4 x i64> %a1) {
  %res = call <4 x i64> @llvm.x86.avx2.vperm2i128(<4 x i64> %a0, <4 x i64> %a1, i8 40)
  ret <4 x i64> %res

; CHECK-LABEL: @perm2i_0x28
; CHECK-NEXT:  shufflevector <4 x i64> <i64 0{{.*}}, <4 x i64> %a1, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
; CHECK-NEXT:  ret <4 x i64>
}

declare <4 x double> @llvm.x86.avx.vperm2f128.pd.256(<4 x double>, <4 x double>, i8) nounwind readnone
declare <8 x float> @llvm.x86.avx.vperm2f128.ps.256(<8 x float>, <8 x float>, i8) nounwind readnone
declare <8 x i32> @llvm.x86.avx.vperm2f128.si.256(<8 x i32>, <8 x i32>, i8) nounwind readnone
declare <4 x i64> @llvm.x86.avx2.vperm2i128(<4 x i64>, <4 x i64>, i8) nounwind readnone

