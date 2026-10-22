// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.*}}, i32 4095)

cbuffer Foo1 : register(b5)
{
  float arr[4096];
}

float4 main() : SV_TARGET
{
  return arr[4095];
}
