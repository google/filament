// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Make sure NRVO works with constant initializer for return value
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 2.500000e-01)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 5.000000e-01)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 7.500000e-01)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 1.000000e+00)

struct VSOUT {
  float4 pos : SV_Position;
};

VSOUT main() {
  const VSOUT Out = {float4(0.25, 0.5, 0.75, 1.0)};
  return Out;
}
