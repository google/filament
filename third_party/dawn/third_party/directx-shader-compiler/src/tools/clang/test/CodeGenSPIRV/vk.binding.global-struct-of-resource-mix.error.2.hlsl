// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct T {
  float2x3 a[4];
};

struct S {
  Texture2D t[2];
  SamplerState s[3];
  T sub[2];
};

float4 tex2D(S x, float2 v) { return x.t[0].Sample(x.s[0], v); }

// CHECK: 16:3: error: global structures containing both resources and non-resources are not supported
S globalS[2];

float4 main() : SV_Target {
  return tex2D(globalS[0], float2(0,0));
}

