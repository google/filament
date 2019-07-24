#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float _21 = {};

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float _23[2];
    for (int _25 = 0; _25 < 2; )
    {
        _23[_25] = 0.0;
        _25++;
        continue;
    }
    float _37;
    if (as_type<uint>(3.0) != 0u)
    {
        _37 = _23[0];
    }
    else
    {
        _37 = _21;
    }
    out.gl_Position = float4(0.0, 0.0, 0.0, _37);
    return out;
}

