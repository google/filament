#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float3 vClip3 [[user(locn0)]];
    float4 vClip4 [[user(locn1)]];
    float2 vClip2 [[user(locn2)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uShadow2D [[texture(0)]], texture1d<float> uSampler1D [[texture(1)]], texture2d<float> uSampler2D [[texture(2)]], texture3d<float> uSampler3D [[texture(3)]], sampler uShadow2DSmplr [[sampler(0)]], sampler uSampler1DSmplr [[sampler(1)]], sampler uSampler2DSmplr [[sampler(2)]], sampler uSampler3DSmplr [[sampler(3)]])
{
    main0_out out = {};
    float4 _17 = in.vClip4;
    float4 _20 = _17;
    _20.z = _17.w;
    out.FragColor = uShadow2D.sample_compare(uShadow2DSmplr, _20.xy / _20.z, _17.z / _20.z);
    out.FragColor = uSampler1D.sample(uSampler1DSmplr, in.vClip2.x / in.vClip2.y).x;
    out.FragColor = uSampler2D.sample(uSampler2DSmplr, in.vClip3.xy / in.vClip3.z).x;
    out.FragColor = uSampler3D.sample(uSampler3DSmplr, _17.xyz / _17.w).x;
    return out;
}

