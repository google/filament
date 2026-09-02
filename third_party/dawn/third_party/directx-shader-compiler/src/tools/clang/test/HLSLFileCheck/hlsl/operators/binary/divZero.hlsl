// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: storeOutput.f32
// CHECK: 0x7FF8000000000000

float3 x;

float4 main() : SV_TARGET {
  float4 z = float4(0,0,0,0);
  float2 m = (z.xy/z.w * x.xy + x.z);
  return float4(m, 1, 1);
}
