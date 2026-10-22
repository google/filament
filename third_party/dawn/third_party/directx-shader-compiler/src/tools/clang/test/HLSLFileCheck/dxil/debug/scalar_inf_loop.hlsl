// RUN: %dxc -T ps_6_0 -E main -Zi -O3 %s | FileCheck %s

// Test for an infinite loop in scalarizer when generating

// CHECK: @main

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

float3 fn(float3 a, float3 b, float3 c) {
  float3 x = (a - b) * a;
  float j = dot(x, c);
  float k = dot(x, c);
  return float3(j, k, 1);
}

float4x4 mat;

float3 main() : SV_Target0 {
  float3 myVar = normalize(mul(float3(1,1,1), (float3x3)mat));
  float3 accum = float3(0,0,0);
  [unroll] for(int i = 0; i < 2 ; i++) {
     [unroll] for(uint j = 0; j < 4 ; j++) {
      float3 ret = fn(float3(1,2,3), float3(0,0,0), myVar);
      accum = lerp(accum, float3(ret.xy, 1), ret.z);
     }
  }
  return accum;
}

