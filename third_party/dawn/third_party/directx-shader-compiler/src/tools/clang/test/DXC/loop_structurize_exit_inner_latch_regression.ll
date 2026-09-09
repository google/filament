; RUN: %dxopt %s -S -loop-unroll,StructurizeLoopExits=1 | FileCheck %s
; RUN: %dxopt %s -S -dxil-loop-unroll,StructurizeLoopExits=1 | FileCheck %s

; CHECK: mul nsw i32
; CHECK: mul nsw i32
; CHECK-NOT: mul nsw i32

; This is a regression test for a crash in loop unroll. When there are multiple
; exits, the compiler will run hlsl::RemoveUnstructuredLoopExits to try to
; avoid unstructured code.
;
; In this test, the compiler will try to unroll the middle loop. The exit edge
; from %land.lhs.true to %if.then will be removed, and %if.end will be split at
; the beginning, and branch to %if.end instead.
;
; Since the new split block at %if.end becomes the new latch of the inner-most
; loop, it needs to be added to the Loop analysis structure of the inner loop.
; However, it was only added to the current middle loop that is being unrolled.

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define void @main(i32 *%arg0, i32 *%arg1, i32 *%arg2) #0 {
entry:
  br label %while.body.3.preheader.lr.ph

while.body.3.preheader.lr.ph.loopexit:            ; preds = %for.inc
  br label %while.body.3.preheader.lr.ph

while.body.3.preheader.lr.ph:                     ; preds = %while.body.3.preheader.lr.ph.loopexit, %entry
  br label %while.body.3.preheader

while.body.3.preheader:                           ; preds = %while.body.3.preheader.lr.ph, %for.inc
  %i.0 = phi i32 [ 0, %while.body.3.preheader.lr.ph ], [ %inc, %for.inc ]
  br label %while.body.3

while.body.3:                                     ; preds = %while.body.3.preheader, %if.end
  %load_arg0 = load i32, i32* %arg0
  %cmp4 = icmp sgt i32 %load_arg0, 0
  br i1 %cmp4, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %while.body.3
  %load_arg1 = load i32, i32* %arg1
  %load_arg2 = load i32, i32* %arg2
  %mul = mul nsw i32 %load_arg2, %load_arg1
  %cmp7 = icmp eq i32 %mul, 10
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  ret void

if.end:                                           ; preds = %land.lhs.true, %while.body.3
  %cmp10 = icmp sle i32 %i.0, 4
  br i1 %cmp10, label %for.inc, label %while.body.3

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.0, 1
  %cmp = icmp slt i32 %inc, 2
  br i1 %cmp, label %while.body.3.preheader, label %while.body.3.preheader.lr.ph.loopexit, !llvm.loop !3
}

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.8.0.14563 (main, 07ce88034-dirty)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.unroll.full"}
