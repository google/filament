; This test tests that a shader with current dxil validator version does not downgrade to 1.4. 
; (The annotate-with-virtual-register pass was erroneously doing just this)

; RUN: %dxopt %s -dxil-annotate-with-virtual-regs -hlsl-dxilemit -S | FileCheck %s

; CHECK: !dx.valver = !{![[VALVER:.*]]}
; CHECK-NOT: ![[VALVER]] = !{i32 1, i32 4}


; GENERATED FROM:
; dxc -Emain -Tcs_6_1


; void main()
; {
; }



;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; no parameters
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; no parameters
; shader hash: bcdf90f13d29df9ebdc77539089a75a6
;
; Pipeline Runtime Information:
;
; Compute Shader
; NumThreads=(1,1,1)
;
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

define void @main() {
  ret void
}

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.entryPoints = !{!4}

!0 = !{!"dxc(private) 1.8.0.4583 (PIX_MemberFunctions, 2f4a01af1-dirty)"}
!1 = !{i32 1, i32 1}
!2 = !{i32 1, i32 8}
!3 = !{!"cs", i32 6, i32 1}
!4 = !{void ()* @main, !"main", null, null, !5}
!5 = !{i32 4, !6}
!6 = !{i32 1, i32 1, i32 1}