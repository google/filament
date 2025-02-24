; RUN: opt %s -analyze -loops | FileCheck -check-prefix=LOOPBEFORE %s
; RUN: opt %s -dxil-remove-unstructured-loop-exits -o %t.bc
; RUN: opt %t.bc -S | FileCheck %s
; RUN: opt %t.bc -analyze -loops | FileCheck -check-prefix=LOOPAFTER %s

; Exit the loop via the false branch.
; Test propagation of the *original* exit condition out of the loop.
; The negated exit condition is also propagated so it can be the condition
; on the new exit block.

; The loop is in SimplifyLoopForm and LCSSA form.

; LOOPBEFORE: Loop at depth 1 containing: %loop_header<header>,%if,%exiting<exiting>,%endif,%loop_latch<latch><exiting>
; LOOPAFTER: Loop at depth 1 containing: %loop_header<header>,%if,%exiting,%endif,%dx.struct_exit.new_exiting<exiting>,%loop_latch<latch><exiting>

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main(i1 %cond) {
entry:
  %var = alloca i32
  store i32 4, i32* %var
  br i1 %cond, label %loop_header, label %end

loop_header:
  store i32 1, i32* %var
  br label %if

;  CHECK:        if:
;  CHECK-NEXT:     br i1 %cond, label %exiting, label %dx.struct_exit.new_exiting
if:
  br i1 %cond, label %exiting, label %loop_latch

; An exiting block that does not dominate the latch block.
; CHECK:         exiting:
; CHECK-NEXT:     %exit_cond = icmp eq i1 %cond, false
; CHECK-NEXT:     %[[NOT_EXIT_COND:[^ ]*]] = xor i1 %exit_cond, true
; CHECK-NEXT:     br label %endif
exiting:
  %exit_cond = icmp eq i1 %cond, 0 ; false
  br i1 %exit_cond, label %endif, label %dedicated_exit

; CHECK:         endif:
; CHECK-NEXT:      %[[PROP1:[^ ]*]] = phi i1 [ %[[NOT_EXIT_COND]], %exiting ]
; CHECK-NEXT:      %[[PROP:[^ ]*]] = phi i1 [ %exit_cond, %exiting ]
; CHECK-NEXT:      br label %dx.struct_exit.new_exiting
; CHECK:         dx.struct_exit.new_exiting:
; CHECK-NEXT:      %[[PROP3:[^ ]*]] = phi i1{{.*}} [ %[[PROP1]], %endif ]
; CHECK-NEXT:      %[[PROP2:[^ ]*]] = phi i1{{.*}} [ %[[PROP]], %endif ]
; CHECK-NEXT:      br i1 %[[PROP3]], label %loop_latch_dedicated_exit, label %loop_latch

endif:
  br label %loop_latch

loop_latch:
  ; 'rotated'
  br i1 %cond, label %loop_latch_dedicated_exit, label %loop_header

; CHECK:         loop_latch_dedicated_exit:
; CHECK-NEXT:      %[[EC_LCSSA:[^ ]*]] = phi i1{{.*}}[ %[[PROP3]], %dx.struct_exit.new_exiting ]
; CHECK-NEXT:      %[[VAL_LCSSA:[^ ]*]] = phi i1{{.*}}[ %[[PROP2]], %dx.struct_exit.new_exiting ]
; CHECK-NEXT:      br i1 %[[EC_LCSSA]], label %dedicated_exit, label %1
; CHECK:         ; <label>:1
; CHECK-NEXT       br label %end

loop_latch_dedicated_exit:
  br label %end

; CHECK:         dedicated_exit:
; CHECK-NEXT:      %exit_cond.lcssa = phi i1{{.*}}[ %[[VAL_LCSSA]], %loop_latch_dedicated_exit ]

dedicated_exit:
  %exit_cond.lcssa = phi i1 [ %exit_cond, %exiting ]
  %exit_cond.ext = zext i1 %exit_cond.lcssa to i32
  %lastval_candidate = mul i32 %exit_cond.ext, 50
  br label %end

; CHECK: %lastval = phi i32 [ 2, %entry ], [ 3, %1 ], [ %lastval_candidate, %dedicated_exit ]
end:
  %lastval = phi i32 [ 2, %entry ], [ 3, %loop_latch_dedicated_exit ], [ %lastval_candidate, %dedicated_exit ]
  store i32 %lastval, i32* %var ; stores 0 because we took the early exit
  ret void
}
