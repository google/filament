// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s
// dxc should use log-mul-exp pattern to implement all scenarios listed below.

// CHECK: Log
// CHECK: Exp
// CHECK: Log
// CHECK: Exp
// CHECK: Log
// CHECK: Exp
// CHECK: Log
// CHECK: Exp
// CHECK: Log
// CHECK: Exp

float main (float4x4 a : A, float b : B, float4 c: C) : SV_Target
{
    float4x4 p1 = {2.0, 2.0, 3.0, 2.0,
                  2.0, 2.0, 2.0, 2.0,
                  2.0, 2.0, 2.0, 2.0,
                  2.0, 2.0, -1.0, 2.0,}; // not a splat vector
    float4 p2 = {2.33, 2.33, 2.33, 2.33}; // a splat vector but not exact
    float p3 = 2.001; // not an exact value
    float p4 = 4294967296.0; // value greater than int max
    float p5 = 7; // exceeds the mulop threshold criteria for float

    return pow(a,p1)[0][0] + pow(b,p2)[0] + pow(a,p3)[0][0] + pow(a,p4)[0][0] + pow(c,p4)[0] + pow(b,p5);
}
