// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure line number is show for resource failed to map.

// CHECK: local resource not guaranteed to map to unique global resource

SamplerState samp1 : register(s5);

float4 main(float2 a : A) : SV_Target {
  Texture2D text2;
  return text2.Sample(samp1, a);
}