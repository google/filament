// RUN: %dxc -E main -T ps_6_0 -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

Texture2D<float4>               r0;
RWTexture3D<float4>             r1;
SamplerState                    r2;
RaytracingAccelerationStructure r3;
RWBuffer<float4>                r4;
ByteAddressBuffer               r5;
RWByteAddressBuffer             r6;
RWStructuredBuffer<float4>      r7;
AppendStructuredBuffer<float4>  r8;

void getResource(out    Texture2D<float4>               a0,
                 out    RWTexture3D<float4>             a1,
                 out    SamplerState                    a2,
                 out    RaytracingAccelerationStructure a3,
                 out    RWBuffer<float4>                a4,
                 out    ByteAddressBuffer               a5,
                 out    RWByteAddressBuffer             a6,
                 out    RWStructuredBuffer<float4>      a7,
                 out    AppendStructuredBuffer<float4>  a8)
{
    a0 = r0;
    a1 = r1;
    a2 = r2;
    a3 = r3;
    a4 = r4;
    a5 = r5;
    a6 = r6;
    a7 = r7;
    a8 = r8;
}

float4 main(): SV_Target
{
    Texture2D<float4>               x0;
    RWTexture3D<float4>             x1;
    SamplerState                    x2;
    RaytracingAccelerationStructure x3;
    RWBuffer<float4>                x4;
    ByteAddressBuffer               x5;
    RWByteAddressBuffer             x6;
    RWStructuredBuffer<float4>      x7;
    AppendStructuredBuffer<float4>  x8;

// CHECK: OpFunctionCall %void %getResource %x0 %x1 %x2 %x3 %x4 %x5 %x6 %x7 %x8
    getResource(x0, x1, x2, x3, x4, x5, x6, x7, x8);

    float4 pos = x4.Load(0);
    return x0.Sample(x2, float2(x6.Load(pos.x), x5.Load(pos.y)));
}
