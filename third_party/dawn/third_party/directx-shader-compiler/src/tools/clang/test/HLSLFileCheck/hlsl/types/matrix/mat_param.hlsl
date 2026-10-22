// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// CHECK: main

// TODO: check test_inout after disable inline.


struct X {
  float2x2 ma[2];
  float4 f;
  row_major float3x3 m;
};

X x0;
X x1;

void test_inout(inout X x, int idx)
{
   x.ma[0] = x.f;
   if (x.f.x > 9) {
     x = x1;
   }
}

float4 main(float4 a : A, float4 b:B) : SV_TARGET
{
  X x = x0;
  test_inout(x, a.x);
  test_inout(x, a.y);
  return b + x.f + x.ma[a.x][a.y][a.z];
}

