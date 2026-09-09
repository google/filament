// RUN: %dxc -HV 2016 -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %{{[a-z0-9]+.*[a-z0-9]*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %{{[a-z0-9]+.*[a-z0-9]*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %{{[a-z0-9]+.*[a-z0-9]*}})
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %{{[a-z0-9]+.*[a-z0-9]*}})

float4 main ( float a : A, float2 b : B, float4 c: C, float4x4 d: D) : SV_Target
{
    return float4(pow(a, 1), pow(b, float2(1.00,1.00))[0], pow(c, float4(1.00,1.00,1.00,1.00))[2], pow(d, 1.00)[1][2]);
}