; Test the static branch probability heuristics for no-return functions.
; RUN: opt < %s -analyze -branch-prob | FileCheck %s

declare void @g1()
declare void @g2()
declare void @g3()
declare void @g4()

define void @test1(i32 %a, i32 %b) {
entry:
  br label %do.body
; CHECK: edge entry -> do.body probability is 16 / 16 = 100%

do.body:
  %i.0 = phi i32 [ 0, %entry ], [ %inc3, %do.end ]
  call void @g1()
  br label %do.body1
; CHECK: edge do.body -> do.body1 probability is 16 / 16 = 100%

do.body1:
  %j.0 = phi i32 [ 0, %do.body ], [ %inc, %do.body1 ]
  call void @g2()
  %inc = add nsw i32 %j.0, 1
  %cmp = icmp slt i32 %inc, %b
  br i1 %cmp, label %do.body1, label %do.end
; CHECK: edge do.body1 -> do.body1 probability is 124 / 128
; CHECK: edge do.body1 -> do.end probability is 4 / 128

do.end:
  call void @g3()
  %inc3 = add nsw i32 %i.0, 1
  %cmp4 = icmp slt i32 %inc3, %a
  br i1 %cmp4, label %do.body, label %do.end5
; CHECK: edge do.end -> do.body probability is 124 / 128
; CHECK: edge do.end -> do.end5 probability is 4 / 128

do.end5:
  call void @g4()
  ret void
}

define void @test2(i32 %a, i32 %b) {
entry:
  %cmp9 = icmp sgt i32 %a, 0
  br i1 %cmp9, label %for.body.lr.ph, label %for.end6
; CHECK: edge entry -> for.body.lr.ph probability is 20 / 32
; CHECK: edge entry -> for.end6 probability is 12 / 32

for.body.lr.ph:
  %cmp27 = icmp sgt i32 %b, 0
  br label %for.body
; CHECK: edge for.body.lr.ph -> for.body probability is 16 / 16 = 100%

for.body:
  %i.010 = phi i32 [ 0, %for.body.lr.ph ], [ %inc5, %for.end ]
  call void @g1()
  br i1 %cmp27, label %for.body3, label %for.end
; CHECK: edge for.body -> for.body3 probability is 20 / 32 = 62.5%
; CHECK: edge for.body -> for.end probability is 12 / 32 = 37.5%

for.body3:
  %j.08 = phi i32 [ %inc, %for.body3 ], [ 0, %for.body ]
  call void @g2()
  %inc = add nsw i32 %j.08, 1
  %exitcond = icmp eq i32 %inc, %b
  br i1 %exitcond, label %for.end, label %for.body3
; CHECK: edge for.body3 -> for.end probability is 4 / 128
; CHECK: edge for.body3 -> for.body3 probability is 124 / 128

for.end:
  call void @g3()
  %inc5 = add nsw i32 %i.010, 1
  %exitcond11 = icmp eq i32 %inc5, %a
  br i1 %exitcond11, label %for.end6, label %for.body
; CHECK: edge for.end -> for.end6 probability is 4 / 128
; CHECK: edge for.end -> for.body probability is 124 / 128

for.end6:
  call void @g4()
  ret void
}

define void @test3(i32 %a, i32 %b, i32* %c) {
entry:
  br label %do.body
; CHECK: edge entry -> do.body probability is 16 / 16 = 100%

do.body:
  %i.0 = phi i32 [ 0, %entry ], [ %inc4, %if.end ]
  call void @g1()
  %0 = load i32, i32* %c, align 4
  %cmp = icmp slt i32 %0, 42
  br i1 %cmp, label %do.body1, label %if.end
; CHECK: edge do.body -> do.body1 probability is 16 / 32 = 50%
; CHECK: edge do.body -> if.end probability is 16 / 32 = 50%

do.body1:
  %j.0 = phi i32 [ %inc, %do.body1 ], [ 0, %do.body ]
  call void @g2()
  %inc = add nsw i32 %j.0, 1
  %cmp2 = icmp slt i32 %inc, %b
  br i1 %cmp2, label %do.body1, label %if.end
; CHECK: edge do.body1 -> do.body1 probability is 124 / 128
; CHECK: edge do.body1 -> if.end probability is 4 / 128

if.end:
  call void @g3()
  %inc4 = add nsw i32 %i.0, 1
  %cmp5 = icmp slt i32 %inc4, %a
  br i1 %cmp5, label %do.body, label %do.end6
; CHECK: edge if.end -> do.body probability is 124 / 128
; CHECK: edge if.end -> do.end6 probability is 4 / 128

do.end6:
  call void @g4()
  ret void
}

define void @test4(i32 %a, i32 %b, i32* %c) {
entry:
  br label %do.body
; CHECK: edge entry -> do.body probability is 16 / 16 = 100%

do.body:
  %i.0 = phi i32 [ 0, %entry ], [ %inc4, %do.end ]
  call void @g1()
  %0 = load i32, i32* %c, align 4
  %cmp = icmp slt i32 %0, 42
  br i1 %cmp, label %return, label %do.body1
; CHECK: edge do.body -> return probability is 4 / 128
; CHECK: edge do.body -> do.body1 probability is 124 / 128

do.body1:
  %j.0 = phi i32 [ %inc, %do.body1 ], [ 0, %do.body ]
  call void @g2()
  %inc = add nsw i32 %j.0, 1
  %cmp2 = icmp slt i32 %inc, %b
  br i1 %cmp2, label %do.body1, label %do.end
; CHECK: edge do.body1 -> do.body1 probability is 124 / 128
; CHECK: edge do.body1 -> do.end probability is 4 / 128

do.end:
  call void @g3()
  %inc4 = add nsw i32 %i.0, 1
  %cmp5 = icmp slt i32 %inc4, %a
  br i1 %cmp5, label %do.body, label %do.end6
; CHECK: edge do.end -> do.body probability is 124 / 128
; CHECK: edge do.end -> do.end6 probability is 4 / 128

do.end6:
  call void @g4()
  br label %return
; CHECK: edge do.end6 -> return probability is 16 / 16 = 100%

return:
  ret void
}

define void @test5(i32 %a, i32 %b, i32* %c) {
entry:
  br label %do.body
; CHECK: edge entry -> do.body probability is 16 / 16 = 100%

do.body:
  %i.0 = phi i32 [ 0, %entry ], [ %inc4, %do.end ]
  call void @g1()
  br label %do.body1
; CHECK: edge do.body -> do.body1 probability is 16 / 16 = 100%

do.body1:
  %j.0 = phi i32 [ 0, %do.body ], [ %inc, %if.end ]
  %0 = load i32, i32* %c, align 4
  %cmp = icmp slt i32 %0, 42
  br i1 %cmp, label %return, label %if.end
; CHECK: edge do.body1 -> return probability is 4 / 128
; CHECK: edge do.body1 -> if.end probability is 124 / 128

if.end:
  call void @g2()
  %inc = add nsw i32 %j.0, 1
  %cmp2 = icmp slt i32 %inc, %b
  br i1 %cmp2, label %do.body1, label %do.end
; CHECK: edge if.end -> do.body1 probability is 124 / 128
; CHECK: edge if.end -> do.end probability is 4 / 128

do.end:
  call void @g3()
  %inc4 = add nsw i32 %i.0, 1
  %cmp5 = icmp slt i32 %inc4, %a
  br i1 %cmp5, label %do.body, label %do.end6
; CHECK: edge do.end -> do.body probability is 124 / 128
; CHECK: edge do.end -> do.end6 probability is 4 / 128

do.end6:
  call void @g4()
  br label %return
; CHECK: edge do.end6 -> return probability is 16 / 16 = 100%

return:
  ret void
}

define void @test6(i32 %a, i32 %b, i32* %c) {
entry:
  br label %do.body
; CHECK: edge entry -> do.body probability is 16 / 16 = 100%

do.body:
  %i.0 = phi i32 [ 0, %entry ], [ %inc4, %do.end ]
  call void @g1()
  br label %do.body1
; CHECK: edge do.body -> do.body1 probability is 16 / 16 = 100%

do.body1:
  %j.0 = phi i32 [ 0, %do.body ], [ %inc, %do.cond ]
  call void @g2()
  %0 = load i32, i32* %c, align 4
  %cmp = icmp slt i32 %0, 42
  br i1 %cmp, label %return, label %do.cond
; CHECK: edge do.body1 -> return probability is 4 / 128
; CHECK: edge do.body1 -> do.cond probability is 124 / 128

do.cond:
  %inc = add nsw i32 %j.0, 1
  %cmp2 = icmp slt i32 %inc, %b
  br i1 %cmp2, label %do.body1, label %do.end
; CHECK: edge do.cond -> do.body1 probability is 124 / 128
; CHECK: edge do.cond -> do.end probability is 4 / 128

do.end:
  call void @g3()
  %inc4 = add nsw i32 %i.0, 1
  %cmp5 = icmp slt i32 %inc4, %a
  br i1 %cmp5, label %do.body, label %do.end6
; CHECK: edge do.end -> do.body probability is 124 / 128
; CHECK: edge do.end -> do.end6 probability is 4 / 128

do.end6:
  call void @g4()
  br label %return
; CHECK: edge do.end6 -> return probability is 16 / 16 = 100%

return:
  ret void
}

define void @test7(i32 %a, i32 %b, i32* %c) {
entry:
  %cmp10 = icmp sgt i32 %a, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.end7
; CHECK: edge entry -> for.body.lr.ph probability is 20 / 32
; CHECK: edge entry -> for.end7 probability is 12 / 32

for.body.lr.ph:
  %cmp38 = icmp sgt i32 %b, 0
  br label %for.body
; CHECK: edge for.body.lr.ph -> for.body probability is 16 / 16 = 100%

for.body:
  %i.011 = phi i32 [ 0, %for.body.lr.ph ], [ %inc6, %for.inc5 ]
  %0 = load i32, i32* %c, align 4
  %cmp1 = icmp eq i32 %0, %i.011
  br i1 %cmp1, label %for.inc5, label %if.end
; CHECK: edge for.body -> for.inc5 probability is 16 / 32 = 50%
; CHECK: edge for.body -> if.end probability is 16 / 32 = 50%

if.end:
  call void @g1()
  br i1 %cmp38, label %for.body4, label %for.end
; CHECK: edge if.end -> for.body4 probability is 20 / 32 = 62.5%
; CHECK: edge if.end -> for.end probability is 12 / 32 = 37.5%

for.body4:
  %j.09 = phi i32 [ %inc, %for.body4 ], [ 0, %if.end ]
  call void @g2()
  %inc = add nsw i32 %j.09, 1
  %exitcond = icmp eq i32 %inc, %b
  br i1 %exitcond, label %for.end, label %for.body4
; CHECK: edge for.body4 -> for.end probability is 4 / 128
; CHECK: edge for.body4 -> for.body4 probability is 124 / 128

for.end:
  call void @g3()
  br label %for.inc5
; CHECK: edge for.end -> for.inc5 probability is 16 / 16 = 100%

for.inc5:
  %inc6 = add nsw i32 %i.011, 1
  %exitcond12 = icmp eq i32 %inc6, %a
  br i1 %exitcond12, label %for.end7, label %for.body
; CHECK: edge for.inc5 -> for.end7 probability is 4 / 128
; CHECK: edge for.inc5 -> for.body probability is 124 / 128

for.end7:
  call void @g4()
  ret void
}

define void @test8(i32 %a, i32 %b, i32* %c) {
entry:
  %cmp18 = icmp sgt i32 %a, 0
  br i1 %cmp18, label %for.body.lr.ph, label %for.end15
; CHECK: edge entry -> for.body.lr.ph probability is 20 / 32
; CHECK: edge entry -> for.end15 probability is 12 / 32

for.body.lr.ph:
  %cmp216 = icmp sgt i32 %b, 0
  %arrayidx5 = getelementptr inbounds i32, i32* %c, i64 1
  %arrayidx9 = getelementptr inbounds i32, i32* %c, i64 2
  br label %for.body
; CHECK: edge for.body.lr.ph -> for.body probability is 16 / 16 = 100%

for.body:
  %i.019 = phi i32 [ 0, %for.body.lr.ph ], [ %inc14, %for.end ]
  call void @g1()
  br i1 %cmp216, label %for.body3, label %for.end
; CHECK: edge for.body -> for.body3 probability is 20 / 32 = 62.5%
; CHECK: edge for.body -> for.end probability is 12 / 32 = 37.5%

for.body3:
  %j.017 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  %0 = load i32, i32* %c, align 4
  %cmp4 = icmp eq i32 %0, %j.017
  br i1 %cmp4, label %for.inc, label %if.end
; CHECK: edge for.body3 -> for.inc probability is 16 / 32 = 50%
; CHECK: edge for.body3 -> if.end probability is 16 / 32 = 50%

if.end:
  %1 = load i32, i32* %arrayidx5, align 4
  %cmp6 = icmp eq i32 %1, %j.017
  br i1 %cmp6, label %for.inc, label %if.end8
; CHECK: edge if.end -> for.inc probability is 16 / 32 = 50%
; CHECK: edge if.end -> if.end8 probability is 16 / 32 = 50%

if.end8:
  %2 = load i32, i32* %arrayidx9, align 4
  %cmp10 = icmp eq i32 %2, %j.017
  br i1 %cmp10, label %for.inc, label %if.end12
; CHECK: edge if.end8 -> for.inc probability is 16 / 32 = 50%
; CHECK: edge if.end8 -> if.end12 probability is 16 / 32 = 50%

if.end12:
  call void @g2()
  br label %for.inc
; CHECK: edge if.end12 -> for.inc probability is 16 / 16 = 100%

for.inc:
  %inc = add nsw i32 %j.017, 1
  %exitcond = icmp eq i32 %inc, %b
  br i1 %exitcond, label %for.end, label %for.body3
; CHECK: edge for.inc -> for.end probability is 4 / 128
; CHECK: edge for.inc -> for.body3 probability is 124 / 128

for.end:
  call void @g3()
  %inc14 = add nsw i32 %i.019, 1
  %exitcond20 = icmp eq i32 %inc14, %a
  br i1 %exitcond20, label %for.end15, label %for.body
; CHECK: edge for.end -> for.end15 probability is 4 / 128
; CHECK: edge for.end -> for.body probability is 124 / 128

for.end15:
  call void @g4()
  ret void
}
