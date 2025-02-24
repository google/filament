// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: @main
struct C {
    float4 c;
};

struct D {
    float2 x[2];
    float3 y[2];
};

float4 main(float4 a : A, inout float4 b : B, inout C c : X, out C d:Y, out float e:O, out float2 f:P, D m[2] : N) : SV_POSITION
{
  d.c = a;
  e = a.z;
  D n = m[a.x];
  f = a.yw + n.x[a.y] + n.y[a.z].zx;
  return abs(a*a.yxxx);
}

