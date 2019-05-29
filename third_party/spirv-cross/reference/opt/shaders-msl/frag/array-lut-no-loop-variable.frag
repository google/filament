#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float _17[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    for (int _46 = 0; _46 < 4; )
    {
        int _33 = _46 + 1;
        out.FragColor += float4(_17[_33]);
        _46 = _33;
        continue;
    }
    return out;
}

