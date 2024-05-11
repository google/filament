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
    int _10 = {};
    int _15 = {};
    for (int _16 = 0, _17 = 0; _16 < in.vA; _17 = _15, _16 += _10)
    {
        if ((in.vA + _16) == 20)
        {
            _15 = 50;
        }
        else
        {
            _15 = ((in.vB + _16) == 40) ? 60 : _17;
        }
        _10 = _15 + 10;
        out.FragColor += float4(1.0);
    }
    return out;
}

