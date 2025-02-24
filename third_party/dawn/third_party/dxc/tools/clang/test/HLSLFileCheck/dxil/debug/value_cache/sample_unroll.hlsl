// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// When we sample texture in a loop, in effort to legalize
// the offsets, an unroll will be done. This tests that
// it still works in Od.

// CHECK: @main
// CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60
// CHECK-SAME: i32 -1, i32 -1
// CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60
// CHECK-SAME: i32 0, i32 0
// CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60
// CHECK-SAME: i32 1, i32 1
// CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60
// CHECK-SAME: i32 2, i32 2

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState samp0 : register(s0);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1)), DescriptorTable(Sampler(s0))")]
float4 main(float2 uv : TEXCOORD) : SV_Target {

  float4 ret = float4(0,0,0,0);
  for (int i = -1; i < 3; i++) {
    ret += tex0.Sample(samp0, uv, int2(i,i));
  }

  return ret;
}

