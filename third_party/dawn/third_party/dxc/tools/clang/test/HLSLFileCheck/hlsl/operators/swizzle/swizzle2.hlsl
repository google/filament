// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: storeOutput
// CHECK: i8 0, float 2.000000e+00
// CHECK: storeOutput
// CHECK: i8 1, float 3.000000e+00
// CHECK: storeOutput
// CHECK: i8 2, float 8.000000e+00
// CHECK: storeOutput
// CHECK: i8 3, float 7.000000e+00

void test(out float b) {
  b = 2;
}

void test2(out float b) {
  b = 3;
}

void test3(out float2 b) {
   b = float2(7, 8);
}

float4 main(float4 a : A, float4 b : B, float2 c : C) : SV_TARGET
{
  float2x2 m = {a};
  test(m._m00);
  test2(m[1].y);
  test3(m._m01_m10);
  float4 result = float4(m[0].x, m._m11, m._m10_m01);
  return result;
}
