; RUN: opt < %s -correlated-propagation -S | FileCheck %s

; CHECK-LABEL: @simple(
define i8 @simple(i1) {
entry:
  %s = select i1 %0, i8 0, i8 1
  br i1 %0, label %then, label %else

then:
; CHECK: ret i8 0
  %a = phi i8 [ %s, %entry ]
  ret i8 %a

else:
; CHECK: ret i8 1
  %b = phi i8 [ %s, %entry ]
  ret i8 %b
}

; CHECK-LABEL: @loop(
define void @loop(i32) {
entry:
  br label %loop

loop:
  %idx = phi i32 [ %0, %entry ], [ %sel, %loop ]
; CHECK: %idx = phi i32 [ %0, %entry ], [ %2, %loop ]
  %1 = icmp eq i32 %idx, 0
  %2 = add i32 %idx, -1
  %sel = select i1 %1, i32 0, i32 %2
  br i1 %1, label %out, label %loop

out:
  ret void
}

; CHECK-LABEL: @not_correlated(
define i8 @not_correlated(i1, i1) {
entry:
  %s = select i1 %0, i8 0, i8 1
  br i1 %1, label %then, label %else

then:
; CHECK: ret i8 %s
  %a = phi i8 [ %s, %entry ]
  ret i8 %a

else:
; CHECK: ret i8 %s
  %b = phi i8 [ %s, %entry ]
  ret i8 %b
}

@c = global i32 0, align 4
@b = global i32 0, align 4

; CHECK-LABEL: @PR23752(
define i32 @PR23752() {
entry:
  br label %for.body

for.body:
  %phi = phi i32 [ 0, %entry ], [ %sel, %for.body ]
  %sel = select i1 icmp sgt (i32* @b, i32* @c), i32 %phi, i32 1
  %cmp = icmp ne i32 %sel, 1
  br i1 %cmp, label %for.body, label %if.end

; CHECK:      %[[sel:.*]] = select i1 icmp sgt (i32* @b, i32* @c), i32 0, i32 1
; CHECK-NEXT: %[[cmp:.*]] = icmp ne i32 %[[sel]], 1
; CHECK-NEXT: br i1 %[[cmp]]

if.end:
  ret i32 %sel
; CHECK: ret i32 %[[sel]]
}
