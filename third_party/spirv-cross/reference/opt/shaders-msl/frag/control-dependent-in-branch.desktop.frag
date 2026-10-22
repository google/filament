#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vInput [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = in.vInput;
    float4 _23 = uSampler.sample(uSamplerSmplr, in.vInput.xy);
    float4 _26 = dfdx(in.vInput);
    float4 _29 = dfdy(in.vInput);
    float4 _32 = fwidth(in.vInput);
    float4 _35 = dfdx(in.vInput);
    float4 _38 = dfdy(in.vInput);
    float4 _41 = fwidth(in.vInput);
    float4 _44 = dfdx(in.vInput);
    float4 _47 = dfdy(in.vInput);
    float4 _50 = fwidth(in.vInput);
    if (in.vInput.y > 10.0)
    {
        out.FragColor += _23;
        out.FragColor += _26;
        out.FragColor += _29;
        out.FragColor += _32;
        out.FragColor += _35;
        out.FragColor += _38;
        out.FragColor += _41;
        out.FragColor += _44;
        out.FragColor += _47;
        out.FragColor += _50;
    }
    return out;
}

