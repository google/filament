#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = float4(0.0);
    for (int _43 = 0; _43 < 3; )
    {
        out.FragColor[_43] += float(_43);
        _43++;
        continue;
    }
    return out;
}

