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
    for (int _82 = 0; _82 < 4; )
    {
        float3 _54 = in.aVertex.xyz - float3(_21.lights[_82].Position);
        out.vColor += ((_21.lights[_82].Color * fast::clamp(1.0 - (length(_54) / _21.lights[_82].Radius), 0.0, 1.0)) * dot(in.aNormal, normalize(_54)));
        _82++;
        continue;
    }
    return out;
}

