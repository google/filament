; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; CHECK-DAG: Function: main: error: sync in a non-Compute/Amplification/Mesh/Node Shader must only sync UAV (sync_uglobal).
; CHECK-DAG: note: at 'call void @dx.op.barrierByMemoryHandle(i32 245, %dx.types.Handle %2, i32 3)' in block '#0' of function 'main'.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResBind = type { i32, i32, i32, i8 }
%dx.types.ResourceProperties = type { i32, i32 }
%"class.RWBuffer<vector<float, 4> >" = type { <4 x float> }

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandleFromBinding(i32 217, %dx.types.ResBind { i32 0, i32 0, i32 0, i8 1 }, i32 0, i1 false)  ; CreateHandleFromBinding(bind,index,nonUniformIndex)
  %2 = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %1, %dx.types.ResourceProperties { i32 4106, i32 1033 })  ; AnnotateHandle(res,props)  resource: RWTypedBuffer<4xF32>
  ; GROUP_SYNC | GROUP_SCOPE = 3
  call void @dx.op.barrierByMemoryHandle(i32 245, %dx.types.Handle %2, i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 0.000000e+00)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #0

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryHandle(i32, %dx.types.Handle, i32) #1

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.annotateHandle(i32, %dx.types.Handle, %dx.types.ResourceProperties) #2

; Function Attrs: nounwind readnone
declare %dx.types.Handle @dx.op.createHandleFromBinding(i32, %dx.types.ResBind, i32, i1) #2

attributes #0 = { nounwind }
attributes #1 = { noduplicate nounwind }
attributes #2 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.viewIdState = !{!7}
!dx.entryPoints = !{!8}

!0 = !{!"dxc(private) 1.7.0.4429 (rdat-minsm-check-flags, 9d3b6ba57)"}
!1 = !{i32 1, i32 8}
!2 = !{!"ps", i32 6, i32 8}
!3 = !{null, !4, null, null}
!4 = !{!5}
!5 = !{i32 0, %"class.RWBuffer<vector<float, 4> >"* undef, !"", i32 0, i32 0, i32 1, i32 10, i1 false, i1 false, i1 false, !6}
!6 = !{i32 0, i32 9}
!7 = !{[2 x i32] [i32 0, i32 4]}
!8 = !{void ()* @main, !"main", !9, !3, !14}
!9 = !{null, !10, null}
!10 = !{!11}
!11 = !{i32 0, !"SV_Target", i8 9, i8 16, !12, i8 0, i32 1, i8 4, i32 0, i8 0, !13}
!12 = !{i32 0}
!13 = !{i32 3, i32 15}
!14 = !{i32 0, i64 8589934592}
