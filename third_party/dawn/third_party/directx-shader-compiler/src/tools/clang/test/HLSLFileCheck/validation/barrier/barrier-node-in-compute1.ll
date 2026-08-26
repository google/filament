; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; CHECK-DAG: Function: main: error: sync in a non-Node Shader must not sync node record memory.
; CHECK-DAG: note: at 'call void @dx.op.barrierByMemoryType(i32 244, i32 4, i32 4)' in block '#0' of function 'main'.
; CHECK-DAG: Function: main: error: sync in a non-Node Shader must not sync node record memory.
; CHECK-DAG: note: at 'call void @dx.op.barrierByMemoryType(i32 244, i32 8, i32 2)' in block '#0' of function 'main'.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main() {
  ; Invalid because NODE_INPUT_MEMORY is not applicable in non-Node Shaders
  ; MemoryTypeFlags = NODE_INPUT_MEMORY
  ; SemanticFlags = GROUP_SCOPE
  call void @dx.op.barrierByMemoryType(i32 244, i32 4, i32 4)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
  ; Invalid because NODE_OUTPUT_MEMORY is not applicable in non-Node Shaders
  ; MemoryTypeFlags = NODE_OUTPUT_MEMORY
  ; SemanticFlags = GROUP_SCOPE
  call void @dx.op.barrierByMemoryType(i32 244, i32 8, i32 2)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)
  ret void
}

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryType(i32, i32, i32) #0

attributes #0 = { noduplicate nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.entryPoints = !{!3}

!0 = !{!"dxc(private) 1.7.0.4429 (rdat-minsm-check-flags, 9d3b6ba57)"}
!1 = !{i32 1, i32 8}
!2 = !{!"cs", i32 6, i32 8}
!3 = !{void ()* @main, !"main", null, null, !4}
!4 = !{i32 4, !5}
!5 = !{i32 8, i32 8, i32 1}
