// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32
// Make sure only 1 sample exist.
// CHECK-NOT:call %dx.types.ResRet.f32 @dx.op.sample.f32

Texture2D<float4> tex;
SamplerState ss;

float4 main(float2 uv:UV, float2 a:A) : SV_Target {
  float4 r = 0;
  if (a.x > 0) {
    r = tex.Sample(ss, uv)-1;
  } else {
    if (a.y > 0)
      r = tex.Sample(ss, uv);
    else
      r = tex.Sample(ss, uv) + 3;
  }

  return r;

}