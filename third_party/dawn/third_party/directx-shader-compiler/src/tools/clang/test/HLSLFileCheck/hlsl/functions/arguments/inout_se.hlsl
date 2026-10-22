// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: ; Input signature:
// CHECK: ; C                        0

// CHECK: ; Output signature:
// CHECK: ; C                        0                 linear

// CHECK: loadInput.f32(i32 4, i32 2, i32 0, i8 0, i32 undef
// CHECK: loadInput.f32(i32 4, i32 2, i32 0, i8 1, i32 undef
// CHECK: loadInput.f32(i32 4, i32 2, i32 0, i8 2, i32 undef
// CHECK: loadInput.f32(i32 4, i32 2, i32 0, i8 3, i32 undef
// CHECK: storeOutput.f32(i32 5, i32 0, i32 0, i8 0
// CHECK: storeOutput.f32(i32 5, i32 0, i32 0, i8 1
// CHECK: storeOutput.f32(i32 5, i32 0, i32 0, i8 2
// CHECK: storeOutput.f32(i32 5, i32 0, i32 0, i8 3


void test_out(out float4 m, float4 a)
{
    m = abs(a*a.yxxx);
}

void test_inout(inout float4 m, float4 a)
{
    m = abs(m+a*a.yxxx);
}

float4 main(float4 a : A, float4 b:B, inout float4 c: C) : SV_POSITION
{
  float4 x = b;
  test_inout(x, a);
  test_out(x, b);
  return x;
}

