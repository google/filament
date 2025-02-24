// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure all function still exist.
// CHECK: unused
// CHECK: test_out
// CHECK: test_inout
// CHECK: test

float unused() {
  return 3;
}

void test_out(out float4 m, float4 a)
{
    m = abs(a*a.yxxx);
}

export void test_inout(inout float4 m, float4 a)
{
    m = abs(m+a*a.yxxx);
}

export float4 test(float4 a : A, float4 b:B) : SV_TARGET
{
  float4 x = b;
  test_inout(x, a);
  test_out(x, b);
  return x;
}

