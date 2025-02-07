#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int vA [[user(locn0)]];
    int vB [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = float4(0.0);
    int _49 = {};
    int _58 = {};
    for (int _57 = 0, _60 = 0; _57 < in.vA; _60 = _58, _57 += _49)
    {
        if ((in.vA + _57) == 20)
        {
            _58 = 50;
        }
        else
        {
            _58 = ((in.vB + _57) == 40) ? 60 : _60;
        }
        _49 = _58 + 10;
        out.FragColor += float4(1.0);
    }
    return out;
}

