// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Make sure every row is stored.
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 1, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 2, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 2, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 3, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 3, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 4, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 4, i8 1
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 5, i8 0
// CHECK: dx.op.storeOutput.f32(i32 5, i32 1, i32 5, i8 1



struct Vertex
{
    float2x2 pos     : POSITION0;
    float3x2 t     : T;
};

struct Interpolants
{
    float4 pos     : SV_POSITION0;
    row_major float3x2 p[2] : O;
};

Interpolants main( Vertex In )
{
    Interpolants o;
    o.pos = (float4)In.pos;
    o.p[0] = In.t*2;
    o.p[1] = In.t*3;
    return o;
}