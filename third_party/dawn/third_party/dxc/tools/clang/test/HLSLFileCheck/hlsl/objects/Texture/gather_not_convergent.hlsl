// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 %s  | FileCheck %s
// Test failure expected when run with 19041 SDK DXIL.dll

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

// CHECK: define void @main()

float4 main(float2 coord0 : TEXCOORD0, float2 coord1 : TEXCOORD1) : SV_Target
{
  float2 a = coord0 * coord1;
  uint status;
  float4 r = 0;

// Ensure Gather is allowed under wave-sensitive flow.
// CHECK: br i1
  if (WaveIsFirstLane()) {
// Ensure Gather coord is not considered convergent,
// and fmul of id * factor sinks
// CHECK: fmul
// CHECK: fmul
// CHECK: call %dx.types.ResRet.f32 @dx.op.textureGather.f32
    r += text1.Gather(samp1, a);
  }
// CHECK: br label

// CHECK: call void @dx.op.storeOutput
  return r;
}
