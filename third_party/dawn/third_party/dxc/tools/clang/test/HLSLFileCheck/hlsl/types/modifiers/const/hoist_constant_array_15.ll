; RUN: %opt %s -hlsl-hca -S | FileCheck %s

; Make sure we added the global and removed the alloca
; CHECK: @a.gva.hca = internal unnamed_addr constant [6 x float] [float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00]
; CHECK-NOT: alloca
; CHECK-NOT: store
; CHECK: call void @dx.op.storeOutput.f32

;static const float a[]= { 1, 2, 3, 4};
;uint b;
;float main() : SV_Target {
;  return a[b];
;}

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f:64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.CBufRet.i32 = type { i32, i32, i32, i32 }
%"$Globals" = type { i32 }

@"\01?b@@3IB" = constant i32 0, align 4

define void @main() {
entry:
  %a.gva = alloca [6 x float]
  %gva.init = getelementptr [6 x float], [6 x float]* %a.gva, i32 0, i32 0
  store float 1.000000e+00, float* %gva.init
  %gva.init1 = getelementptr float, float* %gva.init, i32 2
  store float 3.000000e+00, float* %gva.init1
  %gva.init2 = getelementptr float, float* %gva.init1, i32 -1
  store float 2.000000e+00, float* %gva.init2
  %gva.init3 = getelementptr float, float* %gva.init2, i32 2
  store float 4.000000e+00, float* %gva.init3
  %gva.init4 = getelementptr float, float* %gva.init1, i32 2
  store float 5.000000e+00, float* %gva.init4
  %gva.init5 = getelementptr float, float* %gva.init1, i32 3
  store float 6.000000e+00, float* %gva.init5
  %"$Globals_buffer" = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 2, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %0 = call %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32 59, %dx.types.Handle %"$Globals_buffer", i32 0)  ; CBufferLoadLegacy(handle,regIndex)
  %1 = extractvalue %dx.types.CBufRet.i32 %0, 0
  %arrayidx = getelementptr inbounds [6 x float], [6 x float]* %a.gva, i32 0, i32 %1
  %2 = load float, float* %arrayidx, align 4, !tbaa !20
  %arrayidx1 = getelementptr float, float* %arrayidx, i32 1
  %3 = load float, float* %arrayidx1, align 4, !tbaa !20
  %arrayidx2 = getelementptr float, float* %arrayidx1, i32 1
  %4 = load float, float* %arrayidx2, align 4, !tbaa !20
  %arrayidx3 = getelementptr float, float* %arrayidx2, i32 1
  %5 = load float, float* %arrayidx3, align 4, !tbaa !20
  %arrayidx4 = getelementptr float, float* %arrayidx1, i32 3
  %6 = load float, float* %arrayidx4, align 4, !tbaa !20
  %7 = fadd fast float %2, %3
  %8 = fadd fast float %4, %5
  %9 = fadd fast float %6, %7
  %10 = fadd fast float %8, %9
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %10)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

; Function Attrs: nounwind readonly
declare %dx.types.CBufRet.i32 @dx.op.cbufferLoadLegacy.i32(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readonly }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.typeAnnotations = !{!7, !10}
!dx.viewIdState = !{!14}
!dx.entryPoints = !{!15}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 2}
!3 = !{!"ps", i32 6, i32 0}
!4 = !{null, null, !5, null}
!5 = !{!6}
!6 = !{i32 0, %"$Globals"* undef, !"$Globals", i32 0, i32 0, i32 1, i32 4, null}
!7 = !{i32 0, %"$Globals" undef, !8}
!8 = !{i32 4, !9}
!9 = !{i32 6, !"b", i32 3, i32 0, i32 7, i32 5}
!10 = !{i32 1, void ()* @main, !11}
!11 = !{!12}
!12 = !{i32 0, !13, !13}
!13 = !{}
!14 = !{[2 x i32] [i32 0, i32 1]}
!15 = !{void ()* @main, !"main", !16, !4, null}
!16 = !{null, !17, null}
!17 = !{!18}
!18 = !{i32 0, !"SV_Target", i8 9, i8 16, !19, i8 0, i32 1, i8 1, i32 0, i8 0, null}
!19 = !{i32 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"float", !22, i64 0}
!22 = !{!"omnipotent char", !23, i64 0}
!23 = !{!"Simple C/C++ TBAA"}
