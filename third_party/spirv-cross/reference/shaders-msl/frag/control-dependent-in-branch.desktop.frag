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
    float4 t = uSampler.sample(uSamplerSmplr, in.vInput.xy);
    float4 d0 = dfdx(in.vInput);
    float4 d1 = dfdy(in.vInput);
    float4 d2 = fwidth(in.vInput);
    float4 d3 = dfdx(in.vInput);
    float4 d4 = dfdy(in.vInput);
    float4 d5 = fwidth(in.vInput);
    float4 d6 = dfdx(in.vInput);
    float4 d7 = dfdy(in.vInput);
    float4 d8 = fwidth(in.vInput);
    if (in.vInput.y > 10.0)
    {
        out.FragColor += t;
        out.FragColor += d0;
        out.FragColor += d1;
        out.FragColor += d2;
        out.FragColor += d3;
        out.FragColor += d4;
        out.FragColor += d5;
        out.FragColor += d6;
        out.FragColor += d7;
        out.FragColor += d8;
    }
    return out;
}

