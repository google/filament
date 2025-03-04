; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; CHECK-DAG: Function: main: error: sync in a non-Compute/Amplification/Mesh/Node Shader must only sync UAV (sync_uglobal).
; CHECK-DAG: note: at 'call void @dx.op.barrierByMemoryType(i32 244, i32 1, i32 3)' in block '#0' of function 'main'.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main() {
  ; MemoryTypeFlags = UAV_MEMORY
  ; SemanticFlags = GROUP_SYNC | GROUP_SCOPE
  call void @dx.op.barrierByMemoryType(i32 244, i32 1, i32 3)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
  ret void
}

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryType(i32, i32, i32) #1

attributes #0 = { noduplicate nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.viewIdState = !{!3}
!dx.entryPoints = !{!4}

!0 = !{!"dxc(private) 1.7.0.4429 (rdat-minsm-check-flags, 9d3b6ba57)"}
!1 = !{i32 1, i32 8}
!2 = !{!"ps", i32 6, i32 8}
!3 = !{[2 x i32] [i32 0, i32 4]}
!4 = !{void ()* @main, !"main", !5, null, null}
!5 = !{null, !6, null}
!6 = !{!7}
!7 = !{i32 0, !"SV_Target", i8 9, i8 16, !8, i8 0, i32 1, i8 4, i32 0, i8 0, !9}
!8 = !{i32 0}
!9 = !{i32 3, i32 15}
