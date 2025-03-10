; RUN: %dxilver 1.8 | %dxv %s | FileCheck %s

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; CHECK: Function: entry3: error: Call to DXIL intrinsic ThreadId (SV_DispatchThreadID) is not allowed in node shader launch type Coalescing
; CHECK: note: at '%1 = call i32 @dx.op.threadId.i32(i32 93, i32 1)' in block '#0' of function 'entry3'.
; CHECK: Function: entry3: error: Call to DXIL intrinsic GroupId (SV_GroupId) is not allowed in node shader launch type Coalescing
; CHECK: note: at '%2 = call i32 @dx.op.groupId.i32(i32 94, i32 2)' in block '#0' of function 'entry3'.

define void @entry3() {
  %1 = call i32 @dx.op.threadId.i32(i32 93, i32 1)
  %2 = call i32 @dx.op.groupId.i32(i32 94, i32 2)
  %3 = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 1)
  %4 = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
  ret void
}

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.typeAnnotations = !{!4}
!dx.entryPoints = !{!8, !9}

!0 = !{!"dxc(private) 1.7.0.14317 (main, e3c311409675)"}
!1 = !{i32 1, i32 3}
!2 = !{i32 1, i32 8}
!3 = !{!"lib", i32 6, i32 3}
!4 = !{i32 1, void ()* @entry3, !5}
!5 = !{!6}
!6 = !{i32 0, !7, !7}
!7 = !{}
!8 = !{null, !"", null, null, null}
!9 = !{void ()* @entry3, !"entry3", null, null, !10}
!10 = !{i32 8, i32 15, i32 13, i32 2, i32 15, !11, i32 16, i32 -1, i32 4, !12, i32 5, !13}
!11 = !{!"entry3", i32 0}
!12 = !{i32 1, i32 1, i32 1}
!13 = !{i32 0}

; Function Attrs: nounwind readnone
declare i32 @dx.op.threadId.i32(i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.groupId.i32(i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @dx.op.threadIdInGroup.i32(i32, i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32) #0
