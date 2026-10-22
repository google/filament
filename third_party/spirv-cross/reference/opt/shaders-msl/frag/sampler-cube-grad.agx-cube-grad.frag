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

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vTex [[user(locn0), flat]];
};

fragment main0_out main0(main0_in in [[stage_in]], texturecube<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor += uSampler.sample(uSamplerSmplr, in.vTex, spvGradientCube(in.vTex, float3(5.0), float3(8.0)));
    return out;
}

