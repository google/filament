// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-preserve-all-outputs | %FileCheck %s

// CHECK: alloca [2 x float]
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 1, i8 0,
// CHECK-NOT: call void @dx.op.storeOutput.f32

void main(out float a[2] : A, float4 pos : SV_POSITION)
{
    [branch]
    if (pos.x){
        a[0] = pos.x;
        a[1] = pos.y;
        return;
    }

    a[0] = pos.w;
    a[1] = pos.z;
}