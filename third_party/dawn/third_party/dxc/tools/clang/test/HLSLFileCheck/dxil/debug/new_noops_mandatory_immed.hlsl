// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Test that non-const arithmetic are not optimized away and
// do not impact things that require comstant (Like sample offset);

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState samp0 : register(s0);

[RootSignature("DescriptorTable(SRV(t0), SRV(t1)), DescriptorTable(Sampler(s0))")]
float4 main(float2 uv : TEXCOORD) : SV_Target {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  int a = -8;
  // xHECK: %[[preserve_a:.+]] = select i1 %[[p]], i32 -8, i32 -8
  // CHECK: dx.nothing

  int b = 7;
  // xHECK: %[[preserve_b:.+]] = select i1 %[[p]], i32 7, i32 7
  // CHECK: dx.nothing

  int d = a;
  // xHECK: %[[preserve_d:.+]] = select i1 %[[p]], i32 %[[preserve_a]], i32 %[[preserve_a]]
  // CHECK: dx.nothing

  int e = b + a;
  // xHECK: %[[add:.+]] = add

  // CHECK: call %dx.types.ResRet.f32 @dx.op.sample.f32(i32 60, 
  // CHECK-SAME: i32 -8, i32 -1
  return tex0.Sample(samp0, uv, int2(d,e));
}

