// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s
// check that different float literals are being considered for mul-only code gen for pow.

// CHECK: define void @main()
// CHECK-NOT: Log
// CHECK-NOT: Exp

float main ( float a : A, float4x4 b: B, float4 c: C, float2 d: D) : SV_Target
{
    return pow(a, 8.0f) + 
           pow(d, 14.0h)[0] +
           pow(c, 384.0H)[0] +
           pow(c, -32.0F)[0] +
           pow(b, -131072.0L)[0][0] +
           pow(b, 1073741824.0L)[0][0];
}