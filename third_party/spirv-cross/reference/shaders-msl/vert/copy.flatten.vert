#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Light
{
    packed_float3 Position;
    float Radius;
    float4 Color;
};

struct UBO
{
    float4x4 uMVP;
    Light lights[4];
};

struct Light_1
{
    float3 Position;
    float Radius;
    float4 Color;
};

struct main0_out
{
    float4 vColor [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
    float3 aNormal [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _21 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _21.uMVP * in.aVertex;
    out.vColor = float4(0.0);
    Light_1 light;
    for (int i = 0; i < 4; i++)
    {
        light.Position = float3(_21.lights[i].Position);
        light.Radius = _21.lights[i].Radius;
        light.Color = _21.lights[i].Color;
        float3 L = in.aVertex.xyz - light.Position;
        out.vColor += ((_21.lights[i].Color * fast::clamp(1.0 - (length(L) / light.Radius), 0.0, 1.0)) * dot(in.aNormal, fast::normalize(L)));
    }
    return out;
}

