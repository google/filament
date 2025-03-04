; RUN: opt < %s -S -loop-unroll -unroll-runtime=true | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

; Tests for unrolling loops with run-time trip counts

; CHECK: %xtraiter = and i32 %n
; CHECK:  %lcmp.mod = icmp ne i32 %xtraiter, 0
; CHECK:  br i1 %lcmp.mod, label %for.body.prol, label %for.body.preheader.split

; CHECK: for.body.prol:
; CHECK: %indvars.iv.prol = phi i64 [ %indvars.iv.next.prol, %for.body.prol ], [ 0, %for.body.preheader ]
; CHECK:  %prol.iter.sub = sub i32 %prol.iter, 1
; CHECK:  %prol.iter.cmp = icmp ne i32 %prol.iter.sub, 0
; CHECK:  br i1 %prol.iter.cmp, label %for.body.prol, label %for.body.preheader.split, !llvm.loop !0

define i32 @test(i32* nocapture %a, i32 %n) nounwind uwtable readonly {
entry:
  %cmp1 = icmp eq i32 %n, 0
  br i1 %cmp1, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %sum.02 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %sum.02
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}


; Still try to completely unroll loops with compile-time trip counts
; even if the -unroll-runtime is specified

; CHECK: for.body:
; CHECK-NOT: for.body.prol:

define i32 @test1(i32* nocapture %a) nounwind uwtable readonly {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.01 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %sum.01
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add
}

; This is test 2007-05-09-UnknownTripCount.ll which can be unrolled now
; if the -unroll-runtime option is turned on

; CHECK: bb72.2:

define void @foo(i32 %trips) {
entry:
        br label %cond_true.outer

cond_true.outer:
        %indvar1.ph = phi i32 [ 0, %entry ], [ %indvar.next2, %bb72 ]
        br label %bb72

bb72:
        %indvar.next2 = add i32 %indvar1.ph, 1
        %exitcond3 = icmp eq i32 %indvar.next2, %trips
        br i1 %exitcond3, label %cond_true138, label %cond_true.outer

cond_true138:
        ret void
}


; Test run-time unrolling for a loop that counts down by -2.

; CHECK: for.body.prol:
; CHECK: br i1 %prol.iter.cmp, label %for.body.prol, label %for.body.preheader.split

define zeroext i16 @down(i16* nocapture %p, i32 %len) nounwind uwtable readonly {
entry:
  %cmp2 = icmp eq i32 %len, 0
  br i1 %cmp2, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %p.addr.05 = phi i16* [ %incdec.ptr, %for.body ], [ %p, %entry ]
  %len.addr.04 = phi i32 [ %sub, %for.body ], [ %len, %entry ]
  %res.03 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %incdec.ptr = getelementptr inbounds i16, i16* %p.addr.05, i64 1
  %0 = load i16, i16* %p.addr.05, align 2
  %conv = zext i16 %0 to i32
  %add = add i32 %conv, %res.03
  %sub = add nsw i32 %len.addr.04, -2
  %cmp = icmp eq i32 %sub, 0
  br i1 %cmp, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %phitmp = trunc i32 %add to i16
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %res.0.lcssa = phi i16 [ %phitmp, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  ret i16 %res.0.lcssa
}

; Test run-time unrolling disable metadata.
; CHECK: for.body:
; CHECK-NOT: for.body.prol:

define zeroext i16 @test2(i16* nocapture %p, i32 %len) nounwind uwtable readonly {
entry:
  %cmp2 = icmp eq i32 %len, 0
  br i1 %cmp2, label %for.end, label %for.body

for.body:                                         ; preds = %for.body, %entry
  %p.addr.05 = phi i16* [ %incdec.ptr, %for.body ], [ %p, %entry ]
  %len.addr.04 = phi i32 [ %sub, %for.body ], [ %len, %entry ]
  %res.03 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %incdec.ptr = getelementptr inbounds i16, i16* %p.addr.05, i64 1
  %0 = load i16, i16* %p.addr.05, align 2
  %conv = zext i16 %0 to i32
  %add = add i32 %conv, %res.03
  %sub = add nsw i32 %len.addr.04, -2
  %cmp = icmp eq i32 %sub, 0
  br i1 %cmp, label %for.cond.for.end_crit_edge, label %for.body, !llvm.loop !0

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %phitmp = trunc i32 %add to i16
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %res.0.lcssa = phi i16 [ %phitmp, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  ret i16 %res.0.lcssa
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.runtime.disable"}

; CHECK: !0 = distinct !{!0, !1}
; CHECK: !1 = !{!"llvm.loop.unroll.disable"}

