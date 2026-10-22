; REQUIRES: dxil-1-10
; RUN: not %dxv %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Test that an experimental opcode is not allowed in a non-experimental shader model.

; Update instructions for when release shader model is updated:
; After updating ExperimentalNop shader_model to latest experimental in hctdb.py:
; Update metadata to latest released:
; - !1 metadata for DXIL version
; - !2 metadata for shader model

; CHECK: Function: main: error: Use of experimental DXILOpCode requires an experimental shader model.
; CHECK-NEXT: note: at 'call void @dx.op.nop(i32 -2147483648)' in block '#0' of function 'main'.
; CHECK-NEXT: Function: main: error: Entry function performs some operation that is incompatible with the shader stage or other entry properties.  See other errors for details.
; CHECK-NEXT: Function: main: error: Function uses features incompatible with the shader model.
; CHECK-NEXT: Validation failed.

define void @main() {
  call void @dx.op.nop(i32 -2147483648)
  ret void
}

; Function Attrs: nounwind readnone
declare void @dx.op.nop(i32) #0

attributes #0 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!1}
!dx.shaderModel = !{!2}
!dx.resources = !{!3}
!dx.entryPoints = !{!6}

!0 = !{!"custom IR"}
!1 = !{i32 1, i32 9}
!2 = !{!"cs", i32 6, i32 9}
!3 = !{null, null, null, null}
!6 = !{void ()* @main, !"main", null, !3, !7}
!7 = !{i32 0, i64 0, i32 4, !8}
!8 = !{i32 4, i32 1, i32 1}
