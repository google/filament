; RUN: not %dxv %s 2>&1 | FileCheck %s

; Negative tests for the TGSM pointer chain walk added with chained GEP /
; bitcast support. The validator only walks through GEP and bitcast
; operators / instructions; any other construct (phi, select, etc.) in
; the middle of a chain leaves the ultimate base ambiguous and must be
; rejected. Both the phi/select itself (which produces a TGSM pointer
; that is not a GEP or bitcast) and any GEP / bitcast that consumes it
; must be reported.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

@"\01?GA@@3PAIA" = external addrspace(3) global [32 x i32], align 4
@"\01?GB@@3PAIA" = external addrspace(3) global [32 x i32], align 4

define void @main() {
entry:
  %tid = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)
  %cond = icmp ult i32 %tid, 16
  %ga = getelementptr [32 x i32], [32 x i32] addrspace(3)* @"\01?GA@@3PAIA", i32 0, i32 %tid
  %gb = getelementptr [32 x i32], [32 x i32] addrspace(3)* @"\01?GB@@3PAIA", i32 0, i32 %tid

  ; --- select between two TGSM GEPs, then chain a GEP through the select ---
  ; The select itself directly produces a TGSM pointer that is neither a
  ; GEP nor a bitcast, so it must be rejected.
  ; CHECK: error: TGSM pointers must originate from an unambiguous TGSM global variable.
  ; CHECK-NEXT: note: at '{{.*}}select{{.*}}' in block 'entry' of function 'main'.
  %sel = select i1 %cond, i32 addrspace(3)* %ga, i32 addrspace(3)* %gb
  ; A GEP that walks through the select must also be rejected: the chain
  ; walker stops at the select (not a GEP / bitcast / GlobalVariable).
  ; CHECK: error: TGSM pointers must originate from an unambiguous TGSM global variable.
  ; CHECK-NEXT: note: at '%sel_gep = getelementptr i32, i32 addrspace(3)* %sel, i32 0' in block 'entry' of function 'main'.
  %sel_gep = getelementptr i32, i32 addrspace(3)* %sel, i32 0
  store i32 1, i32 addrspace(3)* %sel_gep, align 4

  ; A bitcast that walks through the select must likewise be rejected.
  ; CHECK: error: TGSM pointers must originate from an unambiguous TGSM global variable.
  ; CHECK-NEXT: note: at '%sel_bc = bitcast i32 addrspace(3)* %sel to float addrspace(3)*' in block 'entry' of function 'main'.
  %sel_bc = bitcast i32 addrspace(3)* %sel to float addrspace(3)*
  store float 2.000000e+00, float addrspace(3)* %sel_bc, align 4

  br i1 %cond, label %then, label %else

then:
  %ga2 = getelementptr [32 x i32], [32 x i32] addrspace(3)* @"\01?GA@@3PAIA", i32 0, i32 %tid
  br label %merge

else:
  %gb2 = getelementptr [32 x i32], [32 x i32] addrspace(3)* @"\01?GB@@3PAIA", i32 0, i32 %tid
  br label %merge

merge:
  ; --- phi joining two TGSM GEPs, then chain a GEP/bitcast through the phi ---
  ; The phi itself produces an ambiguous TGSM pointer.
  ; CHECK: error: TGSM pointers must originate from an unambiguous TGSM global variable.
  ; CHECK-NEXT: note: at '{{.*}}phi{{.*}}' in block 'merge' of function 'main'.
  %p = phi i32 addrspace(3)* [ %ga2, %then ], [ %gb2, %else ]
  ; A GEP through the phi must be rejected: the chain walk reaches the
  ; phi which is neither a GEP, a bitcast, nor a GlobalVariable.
  ; CHECK: error: TGSM pointers must originate from an unambiguous TGSM global variable.
  ; CHECK-NEXT: note: at '%phi_gep = getelementptr i32, i32 addrspace(3)* %p, i32 0' in block 'merge' of function 'main'.
  %phi_gep = getelementptr i32, i32 addrspace(3)* %p, i32 0
  store i32 3, i32 addrspace(3)* %phi_gep, align 4

  ; Likewise a bitcast through the phi must be rejected, even when the
  ; chain has multiple GEP / bitcast hops before the phi appears.
  ; CHECK: error: TGSM pointers must originate from an unambiguous TGSM global variable.
  ; CHECK-NEXT: note: at '%phi_bc = bitcast i32 addrspace(3)* %phi_gep to float addrspace(3)*' in block 'merge' of function 'main'.
  %phi_bc = bitcast i32 addrspace(3)* %phi_gep to float addrspace(3)*
  store float 4.000000e+00, float addrspace(3)* %phi_bc, align 4

  ; CHECK: Validation failed.
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
