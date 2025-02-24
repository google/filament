// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
  Texture2D t[2];
  SamplerState s[3];
  RWStructuredBuffer<float> rw;
};

float4 tex2D(S x, float2 v) { return x.t[0].Sample(x.s[0], v); }

// CHECK: 12:3: error: global structures containing buffers are not supported
S globalS[2];

float4 main() : SV_Target {
  return tex2D(globalS[0], float2(0,0)) + globalS[1].rw[0];
}

