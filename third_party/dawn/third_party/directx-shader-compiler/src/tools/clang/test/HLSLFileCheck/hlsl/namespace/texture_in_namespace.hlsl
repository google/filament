// RUN: %dxc -T ps_6_0 %s  | FileCheck %s

// CHECK: dx.op.sample.f32
// CHECK: dx.op.sample.f32

namespace Textures {
  Texture2D<float4> Color : register(t0);
}

namespace Samplers {
  SamplerState Point : register(s0);
}

Texture2D<float4> GlobalTex : register(t1);
SamplerState GlobalSamp : register(s1);

float4 main(in float2 uv : TEXCOORD0) : SV_TARGET0
{
 return (Textures::Color.Sample(Samplers::Point, uv) + GlobalTex.Sample(GlobalSamp, uv))/2.0;
}
