; RUN: %dxv %s | FileCheck %s

; CHECK: Recursion is not permitted

; This test originally covered two validator error messages:
; 1. recursion not allowed
; 2. functions with parameters are not allowed
;
; The recursion error is now handled earlier in the pipeline, and so there
; is no coverage for error #2. But we do need the validator to check for this
; in case someone decides to remove this. So instead, we validate the assembly
; we would have had, which we can generate with this command:
;
; dxc -Vd -T ps_6_0 recursive.hlsl
;

; void test_inout(inout float4 m, float4 a) {
;    if (a.x > 1)
;     test_inout(m, a-1);
;   m = abs(m+a*a.yxxx);
; }
;
; float4 main(float4 a : A, float4 b:B) : SV_TARGET {
;  float4 x = b;
;  test_inout(x, a);
;  return x;
; }

;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; A                        0   xyzw        0     NONE   float
; B                        0   xyzw        1     NONE   float
;
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Target                0   xyzw        0   TARGET   float   xyzw
;
;
; Pipeline Runtime Information:
;
; Pixel Shader
; DepthOutput=0
; SampleFrequency=0
;
;
; Input signature:
;
; Name                 Index             InterpMode
; -------------------- ----- ----------------------
; A                        0                 linear
; B                        0                 linear
;
; Output signature:
;
; Name                 Index             InterpMode
; -------------------- ----- ----------------------
; SV_Target                0
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
;
target datalayout = "e-m:e-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "dxil-ms-dx"

; Function Attrs: alwaysinline nounwind
define internal fastcc void @"\01?test_inout@@YAXAAV?$vector@M$03@@V1@@Z"(<4 x float>* nocapture dereferenceable(16) %m, <4 x float> %a) #0 {
entry:
  %a.i0 = extractelement <4 x float> %a, i32 0
  %a.i1 = extractelement <4 x float> %a, i32 1
  %a.i2 = extractelement <4 x float> %a, i32 2
  %a.i3 = extractelement <4 x float> %a, i32 3
  %0 = load <4 x float>, <4 x float>* %m, align 4
  %.i05 = extractelement <4 x float> %0, i32 0
  %.i16 = extractelement <4 x float> %0, i32 1
  %.i27 = extractelement <4 x float> %0, i32 2
  %.i38 = extractelement <4 x float> %0, i32 3
  %1 = alloca <4 x float>, align 4
  %cmp = fcmp ogt float %a.i0, 1.000000e+00
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store <4 x float> %0, <4 x float>* %1, align 4
  %sub.i0 = fadd float %a.i0, -1.000000e+00
  %sub.i1 = fadd float %a.i1, -1.000000e+00
  %sub.i2 = fadd float %a.i2, -1.000000e+00
  %sub.i3 = fadd float %a.i3, -1.000000e+00
  %sub.upto0 = insertelement <4 x float> undef, float %sub.i0, i32 0
  %sub.upto1 = insertelement <4 x float> %sub.upto0, float %sub.i1, i32 1
  %sub.upto2 = insertelement <4 x float> %sub.upto1, float %sub.i2, i32 2
  %sub = insertelement <4 x float> %sub.upto2, float %sub.i3, i32 3
  call fastcc void @"\01?test_inout@@YAXAAV?$vector@M$03@@V1@@Z"(<4 x float>* nonnull dereferenceable(16) %1, <4 x float> %sub)
  %2 = load <4 x float>, <4 x float>* %1, align 4
  %.i0 = extractelement <4 x float> %2, i32 0
  %.i1 = extractelement <4 x float> %2, i32 1
  %.i2 = extractelement <4 x float> %2, i32 2
  %.i3 = extractelement <4 x float> %2, i32 3
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %.0.i0 = phi float [ %.i0, %if.then ], [ %.i05, %entry ]
  %.0.i1 = phi float [ %.i1, %if.then ], [ %.i16, %entry ]
  %.0.i2 = phi float [ %.i2, %if.then ], [ %.i27, %entry ]
  %.0.i3 = phi float [ %.i3, %if.then ], [ %.i38, %entry ]
  %mul.i0 = fmul float %a.i0, %a.i1
  %mul.i2 = fmul float %a.i2, %a.i0
  %mul.i3 = fmul float %a.i3, %a.i0
  %add.i0 = fadd float %mul.i0, %.0.i0
  %add.i1 = fadd float %mul.i0, %.0.i1
  %add.i2 = fadd float %mul.i2, %.0.i2
  %add.i3 = fadd float %mul.i3, %.0.i3
  %FAbs = call float @dx.op.unary.f32(i32 6, float %add.i0)  ; FAbs(value)
  %3 = insertelement <4 x float> undef, float %FAbs, i64 0
  %FAbs2 = call float @dx.op.unary.f32(i32 6, float %add.i1)  ; FAbs(value)
  %4 = insertelement <4 x float> %3, float %FAbs2, i64 1
  %FAbs3 = call float @dx.op.unary.f32(i32 6, float %add.i2)  ; FAbs(value)
  %5 = insertelement <4 x float> %4, float %FAbs3, i64 2
  %FAbs4 = call float @dx.op.unary.f32(i32 6, float %add.i3)  ; FAbs(value)
  %6 = insertelement <4 x float> %5, float %FAbs4, i64 3
  store <4 x float> %6, <4 x float>* %m, align 4
  ret void
}

define void @main() {
entry:
  %0 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %1 = insertelement <4 x float> undef, float %0, i64 0
  %2 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %3 = insertelement <4 x float> %1, float %2, i64 1
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %5 = insertelement <4 x float> %3, float %4, i64 2
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = insertelement <4 x float> %5, float %6, i64 3
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = insertelement <4 x float> undef, float %8, i64 0
  %10 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %11 = insertelement <4 x float> %9, float %10, i64 1
  %12 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %13 = insertelement <4 x float> %11, float %12, i64 2
  %14 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %15 = insertelement <4 x float> %13, float %14, i64 3
  %16 = alloca <4 x float>, align 4
  store <4 x float> %7, <4 x float>* %16, align 4
  call fastcc void @"\01?test_inout@@YAXAAV?$vector@M$03@@V1@@Z"(<4 x float>* nonnull dereferenceable(16) %16, <4 x float> %15)
  %17 = load <4 x float>, <4 x float>* %16, align 4
  %18 = extractelement <4 x float> %17, i64 0
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %18)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %19 = extractelement <4 x float> %17, i64 1
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %19)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %20 = extractelement <4 x float> %17, i64 2
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %20)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  %21 = extractelement <4 x float> %17, i64 3
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %21)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #1

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #2

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #1

attributes #0 = { alwaysinline nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="0" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!12}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 0}
!2 = !{!"ps", i32 6, i32 0}
!3 = !{i32 1, void (<4 x float>*, <4 x float>)* @"\01?test_inout@@YAXAAV?$vector@M$03@@V1@@Z", !4, void ()* @main, !10}
!4 = !{!5, !7, !9}
!5 = !{i32 1, !6, !6}
!6 = !{}
!7 = !{i32 2, !8, !6}
!8 = !{i32 7, i32 9}
!9 = !{i32 0, !8, !6}
!10 = !{!11}
!11 = !{i32 0, !6, !6}
!12 = !{void ()* @main, !"main", !13, null, null}
!13 = !{!14, !18, null}
!14 = !{!15, !17}
!15 = !{i32 0, !"A", i8 9, i8 0, !16, i8 2, i32 1, i8 4, i32 0, i8 0, null}
!16 = !{i32 0}
!17 = !{i32 1, !"B", i8 9, i8 0, !16, i8 2, i32 1, i8 4, i32 1, i8 0, null}
!18 = !{!19}
!19 = !{i32 0, !"SV_Target", i8 9, i8 16, !16, i8 0, i32 1, i8 4, i32 0, i8 0, null}
