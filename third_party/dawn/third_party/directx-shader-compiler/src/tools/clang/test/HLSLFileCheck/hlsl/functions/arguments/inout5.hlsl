// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: loadInput.f32(i32 4
// CHECK: loadInput.f32(i32 4
// CHECK: loadInput.f32(i32 4
// CHECK: loadInput.f32(i32 4

// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5
// CHECK: storeOutput.f32(i32 5


float4 main(inout float4 a : A) : SV_POSITION
{
  return 0;
}

