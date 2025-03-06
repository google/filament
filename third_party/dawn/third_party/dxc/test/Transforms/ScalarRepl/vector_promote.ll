; RUN: opt < %s -scalarrepl -S | FileCheck %s
target datalayout = "E-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64"
target triple = "x86_64-apple-darwin10.0.0"

define void @test1(<4 x float>* %F, float %f) {
entry:
	%G = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%tmp = load <4 x float>, <4 x float>* %F		; <<4 x float>> [#uses=2]
	%tmp3 = fadd <4 x float> %tmp, %tmp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp3, <4 x float>* %G
	%G.upgrd.1 = getelementptr <4 x float>, <4 x float>* %G, i32 0, i32 0		; <float*> [#uses=1]
	store float %f, float* %G.upgrd.1
	%tmp4 = load <4 x float>, <4 x float>* %G		; <<4 x float>> [#uses=2]
	%tmp6 = fadd <4 x float> %tmp4, %tmp4		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp6, <4 x float>* %F
	ret void
; CHECK-LABEL: @test1(
; CHECK-NOT: alloca
; CHECK: %tmp = load <4 x float>, <4 x float>* %F
; CHECK: fadd <4 x float> %tmp, %tmp
; CHECK-NEXT: insertelement <4 x float> %tmp3, float %f, i32 0
}

define void @test2(<4 x float>* %F, float %f) {
entry:
	%G = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=3]
	%tmp = load <4 x float>, <4 x float>* %F		; <<4 x float>> [#uses=2]
	%tmp3 = fadd <4 x float> %tmp, %tmp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp3, <4 x float>* %G
	%tmp.upgrd.2 = getelementptr <4 x float>, <4 x float>* %G, i32 0, i32 2		; <float*> [#uses=1]
	store float %f, float* %tmp.upgrd.2
	%tmp4 = load <4 x float>, <4 x float>* %G		; <<4 x float>> [#uses=2]
	%tmp6 = fadd <4 x float> %tmp4, %tmp4		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp6, <4 x float>* %F
	ret void
; CHECK-LABEL: @test2(
; CHECK-NOT: alloca
; CHECK: %tmp = load <4 x float>, <4 x float>* %F
; CHECK: fadd <4 x float> %tmp, %tmp
; CHECK-NEXT: insertelement <4 x float> %tmp3, float %f, i32 2
}

define void @test3(<4 x float>* %F, float* %f) {
entry:
	%G = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%tmp = load <4 x float>, <4 x float>* %F		; <<4 x float>> [#uses=2]
	%tmp3 = fadd <4 x float> %tmp, %tmp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp3, <4 x float>* %G
	%tmp.upgrd.3 = getelementptr <4 x float>, <4 x float>* %G, i32 0, i32 2		; <float*> [#uses=1]
	%tmp.upgrd.4 = load float, float* %tmp.upgrd.3		; <float> [#uses=1]
	store float %tmp.upgrd.4, float* %f
	ret void
; CHECK-LABEL: @test3(
; CHECK-NOT: alloca
; CHECK: %tmp = load <4 x float>, <4 x float>* %F
; CHECK: fadd <4 x float> %tmp, %tmp
; CHECK-NEXT: extractelement <4 x float> %tmp3, i32 2
}

define void @test4(<4 x float>* %F, float* %f) {
entry:
	%G = alloca <4 x float>, align 16		; <<4 x float>*> [#uses=2]
	%tmp = load <4 x float>, <4 x float>* %F		; <<4 x float>> [#uses=2]
	%tmp3 = fadd <4 x float> %tmp, %tmp		; <<4 x float>> [#uses=1]
	store <4 x float> %tmp3, <4 x float>* %G
	%G.upgrd.5 = getelementptr <4 x float>, <4 x float>* %G, i32 0, i32 0		; <float*> [#uses=1]
	%tmp.upgrd.6 = load float, float* %G.upgrd.5		; <float> [#uses=1]
	store float %tmp.upgrd.6, float* %f
	ret void
; CHECK-LABEL: @test4(
; CHECK-NOT: alloca
; CHECK: %tmp = load <4 x float>, <4 x float>* %F
; CHECK: fadd <4 x float> %tmp, %tmp
; CHECK-NEXT: extractelement <4 x float> %tmp3, i32 0
}

define i32 @test5(float %X) {  ;; should turn into bitcast.
	%X_addr = alloca [4 x float]
        %X1 = getelementptr [4 x float], [4 x float]* %X_addr, i32 0, i32 2
	store float %X, float* %X1
	%a = bitcast float* %X1 to i32*
	%tmp = load i32, i32* %a
	ret i32 %tmp
; CHECK-LABEL: @test5(
; CHECK-NEXT: bitcast float %X to i32
; CHECK-NEXT: ret i32
}

define i64 @test6(<2 x float> %X) {
	%X_addr = alloca <2 x float>
        store <2 x float> %X, <2 x float>* %X_addr
	%P = bitcast <2 x float>* %X_addr to i64*
	%tmp = load i64, i64* %P
	ret i64 %tmp
; CHECK-LABEL: @test6(
; CHECK: bitcast <2 x float> %X to i64
; CHECK: ret i64
}

%struct.test7 = type { [6 x i32] }

define void @test7() {
entry:
  %memtmp = alloca %struct.test7, align 16
  %0 = bitcast %struct.test7* %memtmp to <4 x i32>*
  store <4 x i32> zeroinitializer, <4 x i32>* %0, align 16
  %1 = getelementptr inbounds %struct.test7, %struct.test7* %memtmp, i64 0, i32 0, i64 5
  store i32 0, i32* %1, align 4
  ret void
; CHECK-LABEL: @test7(
; CHECK-NOT: alloca
; CHECK: and i192
}

; When promoting an alloca to a 1-element vector type, instructions that
; produce that same vector type should not be changed to insert one element
; into a new vector. <rdar://problem/14249078>
define <1 x i64> @test8(<1 x i64> %a) {
entry:
  %a.addr = alloca <1 x i64>, align 8
  %__a = alloca <1 x i64>, align 8
  %tmp = alloca <1 x i64>, align 8
  store <1 x i64> %a, <1 x i64>* %a.addr, align 8
  %0 = load <1 x i64>, <1 x i64>* %a.addr, align 8
  store <1 x i64> %0, <1 x i64>* %__a, align 8
  %1 = load <1 x i64>, <1 x i64>* %__a, align 8
  %2 = bitcast <1 x i64> %1 to <8 x i8>
  %3 = bitcast <8 x i8> %2 to <1 x i64>
  %vshl_n = shl <1 x i64> %3, <i64 4>
  store <1 x i64> %vshl_n, <1 x i64>* %tmp
  %4 = load <1 x i64>, <1 x i64>* %tmp
  ret <1 x i64> %4
; CHECK-LABEL: @test8(
; CHECK-NOT: alloca
; CHECK-NOT: insertelement
; CHECK: ret <1 x i64>
}
