; RUN: %opt %s -dxil-loop-deletion,NoSink=1 -S | FileCheck %s

; dxil-loop-deletion runs instcombine internally. Check the no sink flag is set correctly.
; Use NoSink=1 to turn off instruction sinking in instcombine

; CHECK: @main

; Make sure loop is deleted.
; CHECK-NOT: loop:

; CHECK: %sel0 =

; CHECK: if.1:
; CHECK: %sel1 =

; CHECK: if.2:
; CHECK: %sel2 =

; CHECK: if.3:
; CHECK: %sel3 =

; CHECK: if.4:
; CHECK: %sel4 =

; CHECK: if.5:
; CHECK: %sel5 =

; CHECK: if.6:
; CHECK: %sel6 =

define float @main(
  float %a0_, float %a0, float %a1, float %a2, float %a3, float %a4, float %a5, float %a6,
  i1 %cond0, i1 %cond1, i1 %cond2, i1 %cond3, i1 %cond4, i1 %cond5, i1 %cond6,
  i1 %br0, i1 %br1, i1 %br2, i1 %br3, i1 %br4, i1 %br5, i1 %br6,
  i32 %loop_bound)
{
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.inc, %loop ]
  %loop_cond = icmp slt i32 %i, %loop_bound
  %i.inc = add i32 %i, 1
  br i1 %loop_cond, label %loop, label %if.0

if.0:
  %sel0 = select i1 %cond0, float 0.0, float %a0
  br i1 %br0, label %if.1, label %if.end

if.1:
  %sel1 = select i1 %cond1, float %sel0, float %a1
  br i1 %br1, label %if.2, label %if.end

if.2:
  %sel2 = select i1 %cond2, float %sel1, float %a2
  br i1 %br2, label %if.3, label %if.end

if.3:
  %sel3 = select i1 %cond3, float %sel2, float %a3
  br i1 %br3, label %if.4, label %if.end

if.4:
  %sel4 = select i1 %cond4, float %sel3, float %a4
  br i1 %br4, label %if.5, label %if.end

if.5:
  %sel5 = select i1 %cond5, float %sel4, float %a5
  br i1 %br5, label %if.6, label %if.end

if.6:
  %sel6 = select i1 %cond6, float %sel5, float %a6
  br label %if.end

if.end:
  %val = phi float [ %sel6, %if.6 ], [ 0.0, %if.0 ], [ 0.0, %if.1 ], [ 0.0, %if.2 ], [ 0.0, %if.3 ], [ 0.0, %if.4 ], [ 0.0, %if.5 ]
  ret float %val
}