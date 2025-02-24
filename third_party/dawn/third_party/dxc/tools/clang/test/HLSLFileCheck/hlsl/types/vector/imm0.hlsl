// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: float4 m;                                     ; Offset:    0
// CHECK: float2 m3[2];                                 ; Offset:   16

// CHECK: 9.000000e+00
// CHECK: 1.100000e+01
// CHECK: 1.300000e+01
// CHECK: 1.500000e+01

float4 m = float4(4,5,6,7);
static float4 m2 = {8,9,10,11};
const float2 m3[2] = {12, 13,14,15};
double x;
float4 main(float4 a : A) : SV_TARGET {
  return x + float4(1,2,3,4) + m + m2 + m3[0].xyxy + m3[1].xyxy;
}
