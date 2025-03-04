// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure cond ? s0 : s1 as temp works.
// CHECK:= call %dx.types.ResRet.f32 @dx.op.sample.f32
bool getCond() { return true;}

SamplerState s0;
SamplerState s1;

Texture2D<float4> tex;

float4 main(float2 uv : UV) : SV_Target {
  bool cond = getCond();
  return tex.Sample(cond ? s0 : s1, uv);
}