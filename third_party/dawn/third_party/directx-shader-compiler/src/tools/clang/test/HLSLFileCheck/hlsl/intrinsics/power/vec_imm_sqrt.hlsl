// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// Make sure vector immediate sqrt works.
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 5.000000e-01)

float2 main() : SV_TARGET
{
  return sqrt(0.25.xx);
}