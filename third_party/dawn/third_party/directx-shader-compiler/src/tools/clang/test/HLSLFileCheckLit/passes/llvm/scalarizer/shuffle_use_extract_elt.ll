; RUN: opt -S -scalarizer -dce %s | FileCheck %s

; CHECK: %[[B:.+]] = load <2 x float>, <2 x float>* %b, align 4
; CHECK: %[[BX0:.+]] = extractelement <2 x float> %[[B]], i32 0
; CHECK: %[[BY0:.+]] = extractelement <2 x float> %[[B]], i32 1
; CHECK: %[[BX1:.+]] = extractelement <2 x float> %[[B]], i32 0
; CHECK: %[[BY1:.+]] = extractelement <2 x float> %[[B]], i32 1

; CHECK: %[[X:.+]] = insertelement <4 x float> undef, float %[[BX0]], i32 0
; CHECK: %[[Y:.+]] = insertelement <4 x float> %[[X]], float %[[BY0]], i32 1
; CHECK: %[[Z:.+]] = insertelement <4 x float> %[[Y]], float %[[BX1]], i32 2
; CHECK: %[[W:.+]] = insertelement <4 x float> %[[Z]], float %[[BY1]], i32 3
; CHECK: ret <4 x float> %[[W]]

declare void @foo(<2 x float>, <2 x float>* dereferenceable(8))

; Function Attrs: noinline nounwind
define internal <4 x float> @bar(<3 x float> %v) #0 {
entry:
  %0 = alloca <2 x float>
  %b = alloca <2 x float>, align 4
  store <2 x float> zeroinitializer, <2 x float>* %b, align 4
  %1 = insertelement <3 x float> %v, float 1.000000e+00, i32 0
  %2 = shufflevector <3 x float> %1, <3 x float> undef, <2 x i32> <i32 0, i32 1>
  store <2 x float> %2, <2 x float>* %0
  ;call void @foo(<2 x float>* dereferenceable(8) %0, <2 x float>* dereferenceable(8) %b)
  %3 = load <2 x float>, <2 x float>* %b, align 4
  %4 = shufflevector <2 x float> %3, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
  ret <4 x float> %4
}

attributes #0 = { noinline nounwind }

