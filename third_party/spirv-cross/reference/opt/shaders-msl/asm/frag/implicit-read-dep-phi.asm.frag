#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 v0 [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uImage [[texture(0)]], sampler uImageSmplr [[sampler(0)]])
{
    main0_out out = {};
    float phi;
    float4 _36;
    int _51;
    _51 = 0;
    phi = 1.0;
    _36 = float4(1.0, 2.0, 1.0, 2.0);
    for (;;)
    {
        out.FragColor = _36;
        if (_51 < 4)
        {
            if (in.v0[_51] > 0.0)
            {
                float2 _48 = float2(phi);
                _51++;
                phi += 2.0;
                _36 = uImage.sample(uImageSmplr, _48, level(0.0));
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    return out;
}

