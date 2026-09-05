; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s
; 

; shader hash: 736cf97c50b38ecbe5281c2034e9b6c5
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define void @node01() #0 {
  
  ; CHECK: Invalid memory type flags
  ; CHECK: Invalid semantic flags
  call void @dx.op.barrierByMemoryType(i32 244, i32 18, i32 9)  ; BarrierByMemoryType(MemoryTypeFlags,SemanticFlags)

  ret void
}

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrierByMemoryType(i32, i32, i32) #1

attributes #0 = { nounwind }
attributes #1 = { noduplicate nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.typeAnnotations = !{!3}
!dx.entryPoints = !{!7, !8}

!0 = !{!"dxc(private) 1.7.0.4846 (user/jbatista/validate_Barrier_args01, 618a7385b-dirty)"}
!1 = !{i32 1, i32 8}
!2 = !{!"lib", i32 6, i32 8}
!3 = !{i32 1, void ()* @node01, !4}
!4 = !{!5}
!5 = !{i32 1, !6, !6}
!6 = !{}
!7 = !{null, !"", null, null, null}
!8 = !{void ()* @node01, !"node01", null, null, !9}
!9 = !{i32 8, i32 15, i32 13, i32 1, i32 15, !10, i32 16, i32 -1, i32 20, !11, i32 4, !13, i32 5, !14}
!10 = !{!"node01", i32 0}
!11 = !{!12}
!12 = !{i32 1, i32 9}
!13 = !{i32 1024, i32 1, i32 1}
!14 = !{i32 0}