// RUN: %dxc -E main -T ps_6_1 -O0 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_1 -O0 %s | FileCheck %s -check-prefixes=CHECK,CHK16

// CHK16: Note: shader requires additional functionality:
// CHK16-NEXT: Barycentrics

// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 0, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 1, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 2, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 0, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 1, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 2, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 0, i8 2)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 1, i8 2)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 2, i8 2)

struct PSInput
{
    float4 position : SV_POSITION;
    nointerpolation float3 color : COLOR;
};
RWByteAddressBuffer outputUAV : register(u0);
cbuffer constants : register(b0)
{
    float4 g_constants;
}
float4 main(PSInput input) : SV_TARGET
{
    uint cmp = (uint)(g_constants[0]);

    float colorAtV0 = GetAttributeAtVertex(input.color, 0)[cmp];
    float colorAtV1 = GetAttributeAtVertex(input.color, 1)[cmp];
    float colorAtV2 = GetAttributeAtVertex(input.color, 2)[cmp];
    outputUAV.Store(0, asuint(colorAtV0));
    outputUAV.Store(4, asuint(colorAtV1));
    outputUAV.Store(8, asuint(colorAtV2));

    return 1.0;
}
