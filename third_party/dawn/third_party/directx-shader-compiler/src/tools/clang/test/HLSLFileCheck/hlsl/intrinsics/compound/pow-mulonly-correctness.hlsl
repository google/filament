// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s

// Verify the mul-only pattern implemented to support Fxc compatability.

// 2.0^8.0.
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 2.560000e+02)

// 2.0^57.0 = 144115188075855872 (0x4380000000000000)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0x4380000000000000)

// 2.0^-5.0
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 3.125000e-02)

//2.0^17.0
// call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 1.310720e+05)

float4 main () : SV_Target
{
    float x1 = 2.0;
    float p1 = 8.0;

    float4x4 x2 = {2.0, 2.0, 2.0, 2.0,
                           2.0, 2.0, 2.0, 2.0,
                           2.0, 2.0, 2.0, 2.0,
                           2.0, 2.0, 2.0, 2.0};
    float4x4 p2 = {57.0, 57.0, 57.0, 57.0,
                           57.0, 57.0, 57.0, 57.0,
                           57.0, 57.0, 57.0, 57.0,
                           57.0, 57.0, 57.0, 57.0};

    float2 x3 = float2(2.0,2.0);
    float2 p3 = float2(-5.0,-5.0);

    float4 x4 = float4(2.0,2.0,2.0,2.0);
    float4 p4 = float4(17.0,17.0,17.0,17.0);

    return float4(pow(x1, p1), pow(x2, p2)[0][0], pow(x3, p3)[0], pow(x4, p4)[0]);
}