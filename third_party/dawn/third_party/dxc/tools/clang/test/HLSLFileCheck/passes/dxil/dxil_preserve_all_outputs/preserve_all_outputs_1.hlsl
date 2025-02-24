// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-preserve-all-outputs | %FileCheck %s

// CHECK: alloca [3 x float]
// CHECK: store float 1.000000e+00
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 1, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 2, i8 0,

void main(out float a[3] : A, float4 pos : SV_POSITION)
{
    a[1] = 1.0;
}