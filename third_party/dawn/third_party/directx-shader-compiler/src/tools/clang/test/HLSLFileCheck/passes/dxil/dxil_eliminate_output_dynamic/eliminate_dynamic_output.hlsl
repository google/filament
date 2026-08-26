// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-eliminate-output-dynamic | %FileCheck %s

// CHECK-NOT: storeOutput.f32(i32 5, i32 0, i32 %i
// CHECK: storeOutput.f32(i32 5, i32 1, i32 0
// CHECK: storeOutput.f32(i32 5, i32 1, i32 1
// CHECK: storeOutput.f32(i32 5, i32 1, i32 2
// CHECK: storeOutput.f32(i32 5, i32 1, i32 3
// CHECK: storeOutput.f32(i32 5, i32 1, i32 4
// CHECK: storeOutput.f32(i32 5, i32 1, i32 5
// CHECK: storeOutput.f32(i32 5, i32 1, i32 6
// CHECK: storeOutput.f32(i32 5, i32 1, i32 7

int  count;
float4 c[16];

float4 main(out float o[8] : I, float4 pos: POS) : SV_POSITION {

    for (uint i=0;i<count;i++)
        o[i] = c[i].x;

    return pos;
}

