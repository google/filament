; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

; CHECK: Function: main: error: Opcode BarrierByNodeRecordHandle not valid in shader model cs_6_8.
; CHECK: note: at 'call void @dx.op.barrierByNodeRecordHandle(i32 246, %dx.types.NodeRecordHandle zeroinitializer, i32 4)' in block '#0' of function 'main'.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.NodeRecordHandle = type { i8* }

define void @main() {
  call void @dx.op.barrierByNodeRecordHandle(i32 246, %dx.types.NodeRecordHandle zeroinitializer, i32 4)
  ret void
}

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByNodeRecordHandle(i32, %dx.types.NodeRecordHandle, i32) #0

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
