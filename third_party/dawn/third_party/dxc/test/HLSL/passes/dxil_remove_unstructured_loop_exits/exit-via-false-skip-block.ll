; RUN: opt %s -analyze -loops | FileCheck -check-prefix=LOOPBEFORE %s
; RUN: opt %s -dxil-remove-unstructured-loop-exits -o %t.bc
; RUN: opt %t.bc -S | FileCheck %s
; RUN: opt %t.bc -analyze -loops | FileCheck -check-prefix=LOOPAFTER %s

; Exit the loop via the false branch, and test "skip block" logic:
; When propagating values from the current exiting block to the newly-exiting block,
; any blocks along the way that may have side effects have to be guarded by
; whether they would be executed. They should not execute if the original exit
; condition was satisfied.

; The loop is in SimplifyLoopForm and LCSSA form.

; LOOPBEFORE: Loop at depth 1 containing: %loop_header<header>,%if,%exiting<exiting>,%endif,%loop_latch<latch><exiting>
; LOOPAFTER: Loop at depth 1 containing: %loop_header<header>,%if,%exiting,%endif,%dx.struct_exit.cond_body,%dx.struct_exit.cond_end,%dx.struct_exit.new_exiting<exiting>,%loop_latch<latch><exiting>

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float)
declare float @dx.op.unary.f32(i32, float)

define void @main(i1 %cond) {
entry:
  %var = alloca i32
  store i32 4, i32* %var
  %v0 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);  ; DerivX
  br i1 %cond, label %loop_header, label %end

loop_header:
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)
  br label %if

; CHECK:      if:
; CHECK-NEXT:  br i1 %cond, label %exiting, label %[[NEW_EXITING:[^ ]*]]

if:
  br i1 %cond, label %exiting, label %loop_latch

; This first one is not guarded because the exit hasn't been taken yet.
; CHECK:      exiting:
; CHECK-NEXT:  %exit_cond = icmp eq i1 %cond, true
; CHECK-NEXT:  %v1 =
; CHECK-NEXT:  %[[NOT_EXIT_COND:[^ ]*]] = xor i1 %exit_cond, true
; CHECK-NEXT:  br label %endif

exiting:
  %exit_cond = icmp eq i1 %cond, true
  %v1 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);
  br i1 %exit_cond, label %endif, label %dedicated_exit

; The call in traversed blocks must be guarded.
; CHECK:      endif:
; CHECK-NEXT:   %[[PROP:[^ ]*]] = phi i1 [ %[[NOT_EXIT_COND]], %exiting ]
; CHECK-NEXT:   br i1 %[[PROP]], label %[[SKIP_END:[^ ]*]], label %[[SKIP_BODY:[^ ]*]]

; CHECK:      [[SKIP_BODY]]:
; CHECK-NEXT:  %v2 =
; CHECK-NEXT:  br label %[[SKIP_END]]

; CHECK:      [[NEW_EXITING]]:
; CHECK-NEXT:  %[[PROP1:[^ ]*]] = phi i1 [ %[[PROP]], %[[SKIP_END]] ]
; CHECK-NEXT:  br i1 %[[PROP1]], label %loop_latch_dedicated_exit, label %loop_latch

endif:
  %v2 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);
  br label %loop_latch

loop_latch:
  %v3 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);
  br i1 %cond, label %loop_latch_dedicated_exit, label %loop_header

; CHECK: <label>:1
; CHECK_NEXT: %v4 =

loop_latch_dedicated_exit:
  %v4 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);
  br label %end

; CHECK: dedicated_exit:
; CHECK_NEXT: %v5 =

dedicated_exit:
  %v5 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);
  br label %end

; CHECK: end:
; CHECK_NEXT: %v6 =

end:
  %v6 = call float @dx.op.unary.f32(i32 85, float 0.000000e+00);
  ret void
}
