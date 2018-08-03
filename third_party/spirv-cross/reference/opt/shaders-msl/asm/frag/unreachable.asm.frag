#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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
    bool _29;
    for (;;)
    {
        _29 = in.counter == 10;
        if (_29)
        {
            break;
        }
        else
        {
            break;
        }
    }
    bool4 _35 = bool4(_29);
    out.FragColor = float4(_35.x ? float4(10.0).x : float4(30.0).x, _35.y ? float4(10.0).y : float4(30.0).y, _35.z ? float4(10.0).z : float4(30.0).z, _35.w ? float4(10.0).w : float4(30.0).w);
    return out;
}

