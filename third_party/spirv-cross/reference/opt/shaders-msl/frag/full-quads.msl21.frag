#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 outColor [[color(0)]];
};

struct main0_in
{
    float2 inCoords [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> tex1 [[texture(0)]], texture2d<float> tex2 [[texture(1)]], sampler tex1Smplr [[sampler(0)]], sampler tex2Smplr [[sampler(1)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    bool _19 = gl_FragCoord.y < 10.0;
    if (quad_all(_19))
    {
        out.outColor = tex1.sample(tex1Smplr, in.inCoords);
    }
    else
    {
        if (quad_any(_19))
        {
            out.outColor = tex2.sample(tex2Smplr, in.inCoords);
        }
        else
        {
            out.outColor = float4(0.0, 0.0, 0.0, 1.0);
        }
    }
    return out;
}

