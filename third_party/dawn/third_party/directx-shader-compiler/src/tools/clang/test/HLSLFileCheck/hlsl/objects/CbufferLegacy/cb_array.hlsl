// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no lshr created for cbuffer array.
// CHECK-NOT: lshr
// CHECK:[[ID:[^ ]+]] = call i32 @dx.op.loadInput.i32
// CHECK:[[ADD:[^ ]+]] = add nsw i32 [[ID]], 2
// CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %"$Globals_cbuffer", i32 [[ADD]])


float A[6] : register(b0);
float main(int i : A) : SV_TARGET
{
  return A[i] + A[i+1] + A[i+2] ;
}
