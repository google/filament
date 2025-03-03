; RUN: opt < %s -indvars -S | FileCheck %s

; This tests that the IV is not recomputed outside of the loop when it is known
; to be computed by the loop and used in the loop any way. In the example below
; although a's value can be computed outside of the loop, there is no benefit
; in doing so as it has to be computed by the loop anyway.
;
; extern void func(unsigned val);
;
; void test(unsigned m)
; {
;   unsigned a = 0;
;
;   for (int i=0; i<186; i++) {
;     a += m;
;     func(a);
;   }
;
;   func(a);
; }

declare void @func(i32)

; CHECK-LABEL: @test(
define void @test(i32 %m) nounwind uwtable {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %a.05 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %add = add i32 %a.05, %m
; CHECK: tail call void @func(i32 %add)
  tail call void @func(i32 %add)
  %inc = add nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 186
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
; CHECK: for.end:
; CHECK-NOT: mul i32 %m, 186
; CHECK:%add.lcssa = phi i32 [ %add, %for.body ]
; CHECK-NEXT: tail call void @func(i32 %add.lcssa)
  tail call void @func(i32 %add)
  ret void
}

; CHECK-LABEL: @test2(
define i32 @test2(i32 %m) nounwind uwtable {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %a.05 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %add = add i32 %a.05, %m
; CHECK: tail call void @func(i32 %add)
  tail call void @func(i32 %add)
  %inc = add nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 186
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
; CHECK: for.end:
; CHECK-NOT: mul i32 %m, 186
; CHECK:%add.lcssa = phi i32 [ %add, %for.body ]
; CHECK-NEXT: ret i32 %add.lcssa
  ret i32 %add
}
