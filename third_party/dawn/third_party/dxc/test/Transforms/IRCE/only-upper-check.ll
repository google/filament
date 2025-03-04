; RUN: opt -irce -irce-print-range-checks -irce-print-changed-loops %s -S 2>&1 | FileCheck %s

; CHECK: irce: loop has 1 inductive range checks:
; CHECK-NEXT:InductiveRangeCheck:
; CHECK-NEXT:  Kind: RANGE_CHECK_UPPER
; CHECK-NEXT:  Offset: %offset  Scale: 1  Length:   %len = load i32, i32* %a_len_ptr, !range !0
; CHECK-NEXT:  Branch:   br i1 %abc, label %in.bounds, label %out.of.bounds, !prof !1
; CHECK-NEXT: irce: in function incrementing: constrained Loop at depth 1 containing: %loop<header><exiting>,%in.bounds<latch><exiting>

define void @incrementing(i32 *%arr, i32 *%a_len_ptr, i32 %n, i32 %offset) {
 entry:
  %len = load i32, i32* %a_len_ptr, !range !0
  %first.itr.check = icmp sgt i32 %n, 0
  br i1 %first.itr.check, label %loop, label %exit

 loop:
  %idx = phi i32 [ 0, %entry ] , [ %idx.next, %in.bounds ]
  %idx.next = add i32 %idx, 1
  %array.idx = add i32 %idx, %offset
  %abc = icmp slt i32 %array.idx, %len
  br i1 %abc, label %in.bounds, label %out.of.bounds, !prof !1

 in.bounds:
  %addr = getelementptr i32, i32* %arr, i32 %array.idx
  store i32 0, i32* %addr
  %next = icmp slt i32 %idx.next, %n
  br i1 %next, label %loop, label %exit

 out.of.bounds:
  ret void

 exit:
  ret void
}

!0 = !{i32 0, i32 2147483647}
!1 = !{!"branch_weights", i32 64, i32 4}
