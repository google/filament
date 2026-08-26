// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-preserve-all-outputs | %FileCheck %s

// CHECK: alloca [6 x float]
// CHECK-NOT: storeOutput.f32(i32 5, i32 0, i32 %
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 1, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 1, i8 1,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 2, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 2, i8 1,

void main(out float2 a[3] : A, float4 pos : SV_POSITION)
{
    for (int i = 0; i < pos.x; ++i) {
        a[i].x = pos.y + i;
        a[i].y = pos.z + i;
    }
}