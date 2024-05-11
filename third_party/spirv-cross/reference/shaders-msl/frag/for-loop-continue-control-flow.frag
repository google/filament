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
    int i = 0;
    int _36;
    for (;;)
    {
        if (i < 3)
        {
            int a = i;
            out.FragColor[a] += float(i);
            if (false)
            {
                _36 = 1;
            }
            else
            {
                int _41 = i;
                i = _41 + 1;
                _36 = _41;
            }
            continue;
        }
        else
        {
            break;
        }
    }
    return out;
}

