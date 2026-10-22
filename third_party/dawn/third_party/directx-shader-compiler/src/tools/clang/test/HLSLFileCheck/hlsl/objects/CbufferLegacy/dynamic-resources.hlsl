// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// CHECK: cbufferLoad
// CHECK: cbufferLoad
// CHECK: createHandle
// CHECK: createHandle
// CHECK: cbufferLoad
// CHECK: createHandle
// CHECK: cbufferLoad
// CHECK: createHandle
// CHECK: !"uav1", i32 0, i32 3, i32 8
// CHECK: !"buf2", i32 0, i32 11, i32 8

SamplerState samp1[8] : register(s5);
Texture2D<float4> text1[8] : register(t3);
RWByteAddressBuffer uav1[8] : register(u3);
RWStructuredBuffer<float4> buf2[8];

uint texIdx;
uint samplerIdx;
uint bufIdx;

float4 main (float2 a : A) : SV_Target {
  return text1[texIdx].Sample(samp1[samplerIdx], a) + uav1[bufIdx].Load(a.x) + buf2[bufIdx][a.y];
}