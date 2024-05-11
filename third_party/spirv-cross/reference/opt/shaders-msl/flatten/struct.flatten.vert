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
    Light light;
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

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _18 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _18.uMVP * in.aVertex;
    out.vColor = float4(0.0);
    float3 _39 = in.aVertex.xyz - float3(_18.light.Position);
    out.vColor += ((_18.light.Color * fast::clamp(1.0 - (length(_39) / _18.light.Radius), 0.0, 1.0)) * dot(in.aNormal, fast::normalize(_39)));
    return out;
}

