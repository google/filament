// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Regression test for GitHub #1928, where the result of a constant atan2
// would vary based on if it was evaluated as a constant expression during codegen
// or later through operations lowering and constant propagation optimization passes.

float2 main() : OUT
{
  float x = -1.0f;
  float y = 2.0f;
  return float2(
    // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 0x4000468A80000000)
    atan2(2.0f, -1.0f),
    // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0x4000468A80000000)
    atan2(y, x));
}