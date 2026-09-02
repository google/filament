; RUN: %dxv %s 2>&1 | FileCheck %s

; Regression test for TGSM pointer validation when SM 6.9 preserves native
; vectors. The DXIL validator must accept TGSM pointers produced by chained
; getelementptr and/or bitcast instructions whose ultimate base is an
; unambiguous groupshared global variable. Prior to this fix, the validator
; only allowed a single GEP instruction (optionally over a constant-expression
; GEP), and a single bitcast whose operand was a single GEP; arbitrary chains
; of GEP and bitcast instructions were incorrectly reported as "TGSM pointers
; must originate from an unambiguous TGSM global variable" even though they
; were unambiguously rooted at a TGSM global.

; CHECK: Validation succeeded.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

@"\01?GS@@3PAV?$matrix@M$02$02@@A.v" = external addrspace(3) global [64 x <9 x float>], align 4

define void @main() {
  %tid = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)
  ; First-level GEP: indexes the per-thread element of the groupshared
  ; matrix array. Its base is the global, so it is unambiguous.
  %elt = getelementptr [64 x <9 x float>], [64 x <9 x float>] addrspace(3)* @"\01?GS@@3PAV?$matrix@M$02$02@@A.v", i32 0, i32 %tid
  store <9 x float> zeroinitializer, <9 x float> addrspace(3)* %elt, align 4
  ; Second-level GEP: indexes into the per-thread vector. Its base is a
  ; GEP *instruction* (not a constant expression), which previously
  ; failed validation but is still unambiguously rooted at the global.
  %scalar = getelementptr <9 x float>, <9 x float> addrspace(3)* %elt, i32 0, i32 0
  store float 1.000000e+00, float addrspace(3)* %scalar, align 4

  ; Bitcast of a GEP instruction. Chain: BC -> GEP -> Global.
  %bc1 = bitcast <9 x float> addrspace(3)* %elt to <9 x i32> addrspace(3)*
  store <9 x i32> zeroinitializer, <9 x i32> addrspace(3)* %bc1, align 4

  ; GEP through a bitcast through a GEP. Chain: GEP -> BC -> GEP -> Global.
  %bc1_scalar = getelementptr <9 x i32>, <9 x i32> addrspace(3)* %bc1, i32 0, i32 3
  store i32 7, i32 addrspace(3)* %bc1_scalar, align 4

  ; Bitcast of a bitcast. Chain: BC -> BC -> GEP -> Global.
  %bc2 = bitcast <9 x i32> addrspace(3)* %bc1 to <9 x float> addrspace(3)*
  store <9 x float> zeroinitializer, <9 x float> addrspace(3)* %bc2, align 4

  ; Bitcast of a constant-expression GEP. Chain: BC -> ConstGEP -> Global.
  %bcconst = bitcast <9 x float> addrspace(3)* getelementptr inbounds ([64 x <9 x float>], [64 x <9 x float>] addrspace(3)* @"\01?GS@@3PAV?$matrix@M$02$02@@A.v", i32 0, i32 0) to <9 x i32> addrspace(3)*
  store <9 x i32> zeroinitializer, <9 x i32> addrspace(3)* %bcconst, align 4
  ret void
}

declare i32 @dx.op.threadIdInGroup.i32(i32, i32) #0

attributes #0 = { nounwind readnone }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.entryPoints = !{!4}

!0 = !{!"dxc test"}
!1 = !{i32 1, i32 9}
!2 = !{i32 1, i32 10}
!3 = !{!"cs", i32 6, i32 9}
!4 = !{void ()* @main, !"main", null, null, !5}
!5 = !{i32 4, !6}
!6 = !{i32 64, i32 1, i32 1}
