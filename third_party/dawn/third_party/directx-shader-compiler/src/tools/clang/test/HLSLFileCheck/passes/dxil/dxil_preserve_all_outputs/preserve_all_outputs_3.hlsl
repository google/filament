// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-preserve-all-outputs | %FileCheck %s

// CHECK: alloca float
// CHECK: store float 1.000000e+00
// CHECK: store float 2.000000e+00
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0,

void main(out float a : A, float4 pos : SV_POSITION)
{
    if (pos.x)
        a = 1.0;
    else
        a = 2.0;
}