; REQUIRES: dxil-1-8
; RUN: not %dxv %s 2>&1 | FileCheck %s

; The purpose of this test is to verify an error is emitted when pre-sm6.9
; 16bit IsSpecialFloat OpClass is used.

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind readnone
define i1 @"\01?test_isinf@@YA_N$f16@@Z"(half %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call i1 @dx.op.isSpecialFloat.f16(i32 9, half %h)  ; IsInf(value)
  ret i1 %1
}

; Function Attrs: nounwind readnone
define <2 x i1> @"\01?test_isinf2@@YA?AV?$vector@_N$01@@V?$vector@$f16@$01@@@Z"(<2 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 9, <2 x half> %h)  ; IsInf(value)
  ret <2 x i1> %1
}

; Function Attrs: nounwind readnone
define <3 x i1> @"\01?test_isinf3@@YA?AV?$vector@_N$02@@V?$vector@$f16@$02@@@Z"(<3 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 9, <3 x half> %h)  ; IsInf(value)
  ret <3 x i1> %1
}

; Function Attrs: nounwind readnone
define <4 x i1> @"\01?test_isinf4@@YA?AV?$vector@_N$03@@V?$vector@$f16@$03@@@Z"(<4 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 9, <4 x half> %h)  ; IsInf(value)
  ret <4 x i1> %1
}

; Function Attrs: nounwind readnone
define i1 @"\01?test_isnan@@YA_N$f16@@Z"(half %h) #0 {
  ; CHECK-DAG: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call i1 @dx.op.isSpecialFloat.f16(i32 8, half %h)  ; IsNaN(value)
  ret i1 %1
}

; Function Attrs: nounwind readnone
define <2 x i1> @"\01?test_isnan2@@YA?AV?$vector@_N$01@@V?$vector@$f16@$01@@@Z"(<2 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 8, <2 x half> %h)  ; IsNaN(value)
  ret <2 x i1> %1
}

; Function Attrs: nounwind readnone
define <3 x i1> @"\01?test_isnan3@@YA?AV?$vector@_N$02@@V?$vector@$f16@$02@@@Z"(<3 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 8, <3 x half> %h)  ; IsNaN(value)
  ret <3 x i1> %1
}

; Function Attrs: nounwind readnone
define <4 x i1> @"\01?test_isnan4@@YA?AV?$vector@_N$03@@V?$vector@$f16@$03@@@Z"(<4 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 8, <4 x half> %h)  ; IsNaN(value)
  ret <4 x i1> %1
}

; Function Attrs: nounwind readnone
define i1 @"\01?test_isfinite@@YA_N$f16@@Z"(half %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call i1 @dx.op.isSpecialFloat.f16(i32 10, half %h)  ; IsFinite(value)
  ret i1 %1
}

; Function Attrs: nounwind readnone
define <2 x i1> @"\01?test_isfinite2@@YA?AV?$vector@_N$01@@V?$vector@$f16@$01@@@Z"(<2 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 10, <2 x half> %h)  ; IsFinite(value)
  ret <2 x i1> %1
}

; Function Attrs: nounwind readnone
define <3 x i1> @"\01?test_isfinite3@@YA?AV?$vector@_N$02@@V?$vector@$f16@$02@@@Z"(<3 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 10, <3 x half> %h)  ; IsFinite(value)
  ret <3 x i1> %1
}

; Function Attrs: nounwind readnone
define <4 x i1> @"\01?test_isfinite4@@YA?AV?$vector@_N$03@@V?$vector@$f16@$03@@@Z"(<4 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 10, <4 x half> %h)  ; IsFinite(value)
  ret <4 x i1> %1
}

; Function Attrs: nounwind readnone
define i1 @"\01?test_isnormal@@YA_N$f16@@Z"(half %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call i1 @dx.op.isSpecialFloat.f16(i32 11, half %h)  ; IsNormal(value)
  ret i1 %1
}

; Function Attrs: nounwind readnone
define <2 x i1> @"\01?test_isnormal2@@YA?AV?$vector@_N$01@@V?$vector@$f16@$01@@@Z"(<2 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <2 x i1> @dx.op.isSpecialFloat.v2f16(i32 11, <2 x half> %h)  ; IsNormal(value)
  ret <2 x i1> %1
}

; Function Attrs: nounwind readnone
define <3 x i1> @"\01?test_isnormal3@@YA?AV?$vector@_N$02@@V?$vector@$f16@$02@@@Z"(<3 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <3 x i1> @dx.op.isSpecialFloat.v3f16(i32 11, <3 x half> %h)  ; IsNormal(value)
  ret <3 x i1> %1
}

; Function Attrs: nounwind readnone
define <4 x i1> @"\01?test_isnormal4@@YA?AV?$vector@_N$03@@V?$vector@$f16@$03@@@Z"(<4 x half> %h) #0 {
  ; CHECK-DAG: error: 16 bit IsSpecialFloat overloads require Shader Model 6.9 or higher.
  %1 = call <4 x i1> @dx.op.isSpecialFloat.v4f16(i32 11, <4 x half> %h)  ; IsNormal(value)
  ret <4 x i1> %1
}

; Function Attrs: nounwind readnone
declare i1 @dx.op.isSpecialFloat.f16(i32, half) #0

; Function Attrs: nounwind readnone
declare <2 x i1> @dx.op.isSpecialFloat.v2f16(i32, <2 x half>) #0

; Function Attrs: nounwind readnone
declare <3 x i1> @dx.op.isSpecialFloat.v3f16(i32, <3 x half>) #0

; Function Attrs: nounwind readnone
declare <4 x i1> @dx.op.isSpecialFloat.v4f16(i32, <4 x half>) #0

attributes #0 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.entryPoints = !{!3}

!0 = !{!"dxc(private) 1.8.0.14983 (main, 2da0a54f1)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{null, !"", null, null, !4}
!4 = !{i32 0, i64 8388640}
