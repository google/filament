#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float4 _21 = {};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int counter [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 _24;
    _24 = _21;
    float4 _33;
    for (;;)
    {
        if (in.counter == 10)
        {
            _33 = float4(10.0);
            break;
        }
        else
        {
            _33 = float4(30.0);
            break;
        }
    }
    out.FragColor = _33;
    return out;
}

