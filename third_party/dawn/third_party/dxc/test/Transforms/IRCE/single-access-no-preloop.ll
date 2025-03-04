; RUN: opt -irce -S < %s | FileCheck %s

define void @single_access_no_preloop_no_offset(i32 *%arr, i32 *%a_len_ptr, i32 %n) {
 entry:
  %len = load i32, i32* %a_len_ptr, !range !0
  %first.itr.check = icmp sgt i32 %n, 0
  br i1 %first.itr.check, label %loop, label %exit

 loop:
  %idx = phi i32 [ 0, %entry ] , [ %idx.next, %in.bounds ]
  %idx.next = add i32 %idx, 1
  %abc = icmp slt i32 %idx, %len
  br i1 %abc, label %in.bounds, label %out.of.bounds, !prof !1

 in.bounds:
  %addr = getelementptr i32, i32* %arr, i32 %idx
  store i32 0, i32* %addr
  %next = icmp slt i32 %idx.next, %n
  br i1 %next, label %loop, label %exit

 out.of.bounds:
  ret void

 exit:
  ret void
}

; CHECK-LABEL: single_access_no_preloop

; CHECK-LABEL: loop:
; CHECK: br i1 true, label %in.bounds, label %out.of.bounds

; CHECK-LABEL: main.exit.selector:
; CHECK-NEXT: [[continue:%[^ ]+]] = icmp slt i32 %idx.next, %n
; CHECK-NEXT: br i1 [[continue]], label %main.pseudo.exit, label %exit.loopexit

; CHECK-LABEL: main.pseudo.exit:
; CHECK-NEXT: %idx.copy = phi i32 [ 0, %loop.preheader ], [ %idx.next, %main.exit.selector ]
; CHECK-NEXT: %indvar.end = phi i32 [ 0, %loop.preheader ], [ %idx.next, %main.exit.selector ]
; CHECK-NEXT: br label %postloop

; CHECK-LABEL: postloop:
; CHECK-NEXT: br label %loop.postloop

; CHECK-LABEL: loop.postloop:
; CHECK-NEXT: %idx.postloop = phi i32 [ %idx.next.postloop, %in.bounds.postloop ], [ %idx.copy, %postloop ]
; CHECK-NEXT: %idx.next.postloop = add i32 %idx.postloop, 1
; CHECK-NEXT: %abc.postloop = icmp slt i32 %idx.postloop, %len
; CHECK-NEXT: br i1 %abc.postloop, label %in.bounds.postloop, label %out.of.bounds

; CHECK-LABEL: in.bounds.postloop:
; CHECK-NEXT: %addr.postloop = getelementptr i32, i32* %arr, i32 %idx.postloop
; CHECK-NEXT: store i32 0, i32* %addr.postloop
; CHECK-NEXT: %next.postloop = icmp slt i32 %idx.next.postloop, %n
; CHECK-NEXT: br i1 %next.postloop, label %loop.postloop, label %exit.loopexit


define void @single_access_no_preloop_with_offset(i32 *%arr, i32 *%a_len_ptr, i32 %n) {
 entry:
  %len = load i32, i32* %a_len_ptr, !range !0
  %first.itr.check = icmp sgt i32 %n, 0
  br i1 %first.itr.check, label %loop, label %exit

 loop:
  %idx = phi i32 [ 0, %entry ] , [ %idx.next, %in.bounds ]
  %idx.next = add i32 %idx, 1
  %idx.for.abc = add i32 %idx, 4
  %abc = icmp slt i32 %idx.for.abc, %len
  br i1 %abc, label %in.bounds, label %out.of.bounds, !prof !1

 in.bounds:
  %addr = getelementptr i32, i32* %arr, i32 %idx.for.abc
  store i32 0, i32* %addr
  %next = icmp slt i32 %idx.next, %n
  br i1 %next, label %loop, label %exit

 out.of.bounds:
  ret void

 exit:
  ret void
}

; CHECK-LABEL: single_access_no_preloop_with_offset

; CHECK-LABEL: loop.preheader:
; CHECK: [[not_n:[^ ]+]] = sub i32 -1, %n
; CHECK: [[not_safe_range_end:[^ ]+]] = sub i32 3, %len
; CHECK: [[not_exit_main_loop_at_hiclamp_cmp:[^ ]+]] = icmp sgt i32 [[not_n]], [[not_safe_range_end]]
; CHECK: [[not_exit_main_loop_at_hiclamp:[^ ]+]] = select i1 [[not_exit_main_loop_at_hiclamp_cmp]], i32 [[not_n]], i32 [[not_safe_range_end]]
; CHECK: [[exit_main_loop_at_hiclamp:[^ ]+]] = sub i32 -1, [[not_exit_main_loop_at_hiclamp]]
; CHECK: [[exit_main_loop_at_loclamp_cmp:[^ ]+]] = icmp sgt i32 [[exit_main_loop_at_hiclamp]], 0
; CHECK: [[exit_main_loop_at_loclamp:[^ ]+]] = select i1 [[exit_main_loop_at_loclamp_cmp]], i32 [[exit_main_loop_at_hiclamp]], i32 0
; CHECK: [[enter_main_loop:[^ ]+]] = icmp slt i32 0, [[exit_main_loop_at_loclamp]]
; CHECK: br i1 [[enter_main_loop]], label %loop, label %main.pseudo.exit

; CHECK-LABEL: loop:
; CHECK: br i1 true, label %in.bounds, label %out.of.bounds

; CHECK-LABEL: in.bounds:
; CHECK: [[continue_main_loop:[^ ]+]] = icmp slt i32 %idx.next, [[exit_main_loop_at_loclamp]]
; CHECK: br i1 [[continue_main_loop]], label %loop, label %main.exit.selector

; CHECK-LABEL: main.pseudo.exit:
; CHECK:  %idx.copy = phi i32 [ 0, %loop.preheader ], [ %idx.next, %main.exit.selector ]
; CHECK:  br label %postloop

; CHECK-LABEL: loop.postloop:
; CHECK: %idx.postloop = phi i32 [ %idx.next.postloop, %in.bounds.postloop ], [ %idx.copy, %postloop ]

; CHECK-LABEL: in.bounds.postloop:
; CHECK: %next.postloop = icmp slt i32 %idx.next.postloop, %n
; CHECK: br i1 %next.postloop, label %loop.postloop, label %exit.loopexit

!0 = !{i32 0, i32 2147483647}
!1 = !{!"branch_weights", i32 64, i32 4}
