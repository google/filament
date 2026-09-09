// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul

float2 main (float4 x : A) : SV_Target
{
    float2 y = float2(11.0,11.0);
    return pow(x, y);
}