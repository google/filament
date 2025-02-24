; RUN: opt -instcombine -S %s | FileCheck %s

; CHECK-NOT: call i32 @llvm.bswap.i32
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

; Function Attrs: nounwind
define i32 @main(i32 %input) #0 {
  %1 = shl i32 %input, 24
  %2 = and i32 %1, -16777216
  %3 = shl i32 %input, 8
  %4 = and i32 %3, 16711680
  %5 = or i32 %2, %4
  %6 = lshr i32 %input, 8
  %7 = and i32 %6, 65280
  %8 = or i32 %5, %7
  %9 = lshr i32 %input, 24
  %10 = and i32 %9, 255
  %11 = or i32 %8, %10
  ret i32 %11
}

attributes #0 = { nounwind }

!llvm.module.flags = !{!0}
!pauseresume = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{!"hlsl-dxilemit", !"hlsl-dxilload"}
!2 = !{!"dxc(private) 1.7.0.14160 (main, adb2dc70fbd-dirty)"}
