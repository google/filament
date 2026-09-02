// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure N n2[2] lowered into float [2]
// CHECK:[2 x float]

  struct N {
     float n;
  };

class X {
  float2x2 ma[2];
  N n2[2];
  row_major float3x3 m;
  N test_inout(float idx) {
   return n2[idx];
  }
};

X x0;

float4 main(float4 a : A, float4 b:B) : SV_TARGET
{
  X x = x0;
  x.n2[0].n = 1;
  return x.test_inout(a.x).n;
}
