; RUN: %opt %s -dxil-loop-unroll,StructurizeLoopExits=1 -S | FileCheck %s

; CHECK: call float @llvm.sin.f32(
; CHECK: call float @llvm.sin.f32(
; CHECK: call float @llvm.sin.f32(
; CHECK: call float @llvm.sin.f32(
; CHECK-NOT: call float @llvm.sin.f32(

target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

declare float @llvm.sin.f32(float %Val) #0

; Function Attrs: nounwind
define float @main(i32 %cond) #1 {
entry:
  br label %for.body

for.body:
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %sw.epilog ]
  %ret.0 = phi float [ 0.000000e+00, %entry ], [ %add7, %sw.epilog ]
  %add = fadd fast float %ret.0, 1.000000e+00
  switch i32 %cond, label %end [
    i32 0, label %sw.bb
    i32 1, label %sw.bb.3
  ]

sw.bb:
  %add2 = fadd fast float %add, 1.000000e+01
  br label %sw.epilog

sw.bb.3:
  %add4 = fadd fast float %add, 2.000000e+01
  br label %sw.epilog

sw.epilog:
  %ret.1 = phi float [ %add4, %sw.bb.3 ], [ %add2, %sw.bb ]
  %Sin = call float @llvm.sin.f32(float 0.0)
  %add7 = fadd fast float %ret.1, %Sin
  %inc = add nsw i32 %i.0, 1
  %cmp = icmp slt i32 %inc, 4
  br i1 %cmp, label %for.body, label %end, !llvm.loop !3

end:                           ; preds = %sw.epilog, %for.body
  %retval.0 = phi float [ 4.200000e+01, %for.body ], [ %add7, %sw.epilog ]
  ret float %retval.0
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
attributes #2 = { nounwind readonly }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvmdent = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.7.0.14003 (main, a5b0488bc-dirty)"}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.unroll.full"}