; RUN: opt < %s -indvars -S | FileCheck %s

; The inner loop (%header1) has a fixed trip count.
; The indvars pass is tempted to delete the phi instruction %hexit,
; and replace its uses with %add3.
; But %hexit is used in %latch0, which is outside the inner loop and
; its exit block. Deleting the phi %hexit would break LCSSA form.

; CHECK: @main
; CHECK: exit1:
; CHECK-NEXT: %hexit = phi i32 [ %hnext, %header1 ]
; CHECK-NEXT: br label %latch0

; CHECK: latch0:

target triple = "dxil-ms-dx"

define void @main(i32 %arg) {
entry:
  br label %header0

header0:
  %isgt0 = icmp sgt i32 %arg, 0
  %smax = select i1 %isgt0, i32 %arg, i32 0
  %h0 = add i32 %smax, 1
  %j0 = add i32 %smax, 2
  %doinner = icmp slt i32 %j0, 1
  br i1 %doinner, label %header1.pre, label %latch0

header1.pre:
  br label %header1

header1:
  %hi = phi i32 [ %hnext, %header1 ], [ %h0, %header1.pre ]
  %ji = phi i32 [ %jnext, %header1 ], [ %j0, %header1.pre ]
  %add3 = add i32 %smax, 3
  %hnext = add i32 %hi, 1
  %jnext = add nsw i32 %ji, 1 ; the nsw here is essential
  %do1again = icmp slt i32 %ji, %add3
  br i1 %do1again, label %header1, label %exit1

exit1:
  %hexit = phi i32 [ %hnext, %header1 ]
  br label %latch0

latch0:
  %useh = phi i32 [ %h0, %header0 ], [ %hexit, %exit1 ]
  br label %header0
}
