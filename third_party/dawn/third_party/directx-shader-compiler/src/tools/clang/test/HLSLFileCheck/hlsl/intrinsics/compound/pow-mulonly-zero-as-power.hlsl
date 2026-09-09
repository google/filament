// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 1.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 1.000000e+00)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 1.000000e+00)

float4 main ( float a : A, float2 b : B, float4 c: C, float4x4 d: D) : SV_Target
{
    return float4(pow(a, 0), pow(b, float2(0.00,0.00))[0], pow(c, -0.00)[2], pow(d, 0.00)[1][2]);
}