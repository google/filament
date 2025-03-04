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
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fdiv
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul
// CHECK: fmul

float4 main (float x1 : A, float4x4 x2 : B, float2 x3 : C, float4 x4 : D) : SV_Target
{
    float p1 = 8.0;
    float4x4 p2 =         {57.0, 57.0, 57.0, 57.0,
                           57.0, 57.0, 57.0, 57.0,
                           57.0, 57.0, 57.0, 57.0,
                           57.0, 57.0, 57.0, 57.0};
    float2 p3 = float2(-5.0,-5.0);
    float4 p4 = float4(17.0,17.0,17.0,17.0);

    return float4(pow(x1, p1), pow(x2, p2)[0][0], pow(x3, p3)[0], pow(x4, p4)[0]);
}