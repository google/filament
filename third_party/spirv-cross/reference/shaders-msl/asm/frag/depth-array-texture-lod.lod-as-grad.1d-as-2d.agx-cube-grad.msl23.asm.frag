#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

static inline gradientcube spvGradientCube(float3 P, float3 dPdx, float3 dPdy)
{
    // Major axis selection
    float3 absP = abs(P);
    bool xMajor = absP.x >= max(absP.y, absP.z);
    bool yMajor = absP.y >= absP.z;
    float3 Q = xMajor ? P.yzx : (yMajor ? P.xzy : P);
    float3 dQdx = xMajor ? dPdx.yzx : (yMajor ? dPdx.xzy : dPdx);
    float3 dQdy = xMajor ? dPdy.yzx : (yMajor ? dPdy.xzy : dPdy);

    // Skip a couple of operations compared to usual projection
    float4 d = float4(dQdx.xy, dQdy.xy) - (Q.xy / Q.z).xyxy * float4(dQdx.zz, dQdy.zz);

    // Final swizzle to put the intermediate values into non-ignored components
    // X major: X and Z
    // Y major: X and Y
    // Z major: Y and Z
    return gradientcube(xMajor ? d.xxy : d.xyx, xMajor ? d.zzw : d.zwz);
}

struct buf0
{
    float4 u_scale;
};

struct buf1
{
    float4 u_bias;
};

struct main0_out
{
    float4 o_color [[color(0)]];
};

struct main0_in
{
    float4 v_texCoord [[user(locn0)]];
    float2 v_drefLodBias [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depthcube_array<float> u_sampler [[texture(0)]], sampler u_samplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.o_color = float4(u_sampler.sample_compare(u_samplerSmplr, in.v_texCoord.xyz, uint(rint(in.v_texCoord.w)), in.v_drefLodBias.x, spvGradientCube(in.v_texCoord.xyz, exp2(in.v_drefLodBias.y - 0.5) / float3(u_sampler.get_width()), exp2(in.v_drefLodBias.y - 0.5) / float3(u_sampler.get_width()))), 0.0, 0.0, 1.0);
    return out;
}

