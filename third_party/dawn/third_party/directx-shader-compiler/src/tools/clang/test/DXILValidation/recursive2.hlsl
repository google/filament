// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: error: recursive functions not allowed

struct M {
  float m;
};

void test_inout(inout M m, float a)
{
    if (a.x > 1)
      test_inout(m, a-1);
    m.m = abs(m.m+a);
}

float4 main(float a : A, float b:B) : SV_TARGET
{
  M m;
  m.m = b;
  test_inout(m, a);
  return m.m;
}

