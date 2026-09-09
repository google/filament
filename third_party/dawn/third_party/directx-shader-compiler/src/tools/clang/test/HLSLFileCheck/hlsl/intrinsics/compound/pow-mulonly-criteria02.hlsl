// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s
// dxc should use log-mul-exp pattern to implement all scenarios listed below.

// CHECK: define void @main()
// CHECK-NOT: Log
// CHECK-NOT: Exp

float main (float4x4 a : A, float b : B, float4 c: C) : SV_Target
{
    float4x4 p1 = {2.0, 2.0, 2.0, 2.0,
                  2.0, 2.0, 2.0, 2.0,
                  2.0, 2.0, 2.0, 2.0,
                  2.0, 2.0, 2.0, 2.0,}; // a splat
    float4 p2 = {9, 9, 9, 9}; // another splat
    float p3 = 8; // meets the threshold criteria

    return pow(a,p1)[0][0] + pow(b,p2)[0] + pow(a,p3)[0][0];
}
