; REQUIRES: dxil-1-10
; RUN: %dxa %s -o %t.dxil | FileCheck %s -check-prefix=DXA
; RUN: %dxc -dumpbin %t.dxil | FileCheck %s -check-prefix=DXIL
; RUN: %dxv %t.dxil -o %t.hash.dxil 2>&1 | FileCheck %s -check-prefix=VAL
; RUN: %dxa %t.hash.dxil -dumphash | FileCheck %s -check-prefix=HASH

; DXA: Assembly succeeded.

; DXIL: call void @dx.op.nop(i32 -2147483648)  ; ExperimentalNop()
; DXIL: declare void @dx.op.nop(i32) #[[ATTR:[0-9]+]]
; DXIL: attributes #[[ATTR]] = { nounwind readnone }

; VAL: Validation succeeded.

; Make sure it's the PREVIEW hash.
; HASH: Validation hash: 0x02020202020202020202020202020202

target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

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
!dx.entryPoints = !{!4}

!0 = !{!"custom IR"}
!1 = !{i32 1, i32 10}
!2 = !{!"cs", i32 6, i32 10}
!3 = !{null, null, null, null}
!4 = !{void ()* @main, !"main", null, !3, !5}
!5 = !{i32 0, i64 0, i32 4, !6}
!6 = !{i32 4, i32 1, i32 1}
