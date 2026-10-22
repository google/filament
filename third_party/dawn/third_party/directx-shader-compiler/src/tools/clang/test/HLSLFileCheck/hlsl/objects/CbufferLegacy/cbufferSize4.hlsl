// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.*}}, i32 4095)

cbuffer Foo1 : register(b5)
{
  float arr[4096];
  float3 after_array;
}

float4 main() : SV_TARGET
{
  return float4(arr[4095], after_array);
}
