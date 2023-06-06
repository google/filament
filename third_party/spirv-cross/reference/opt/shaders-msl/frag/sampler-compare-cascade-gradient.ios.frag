#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float4 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d_array<float> uTex [[texture(0)]], sampler uShadow [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uTex.sample_compare(uShadow, in.vUV.xy, uint(rint(in.vUV.z)), in.vUV.w, gradient2d(float2(0.0), float2(0.0)));
    return out;
}

