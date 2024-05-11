#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    int FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = 16;
    for (int i = 0; i < 25; i++)
    {
        out.FragColor += 10;
    }
    for (int i_1 = 1, j = 4; i_1 < 30; i_1++, j += 4)
    {
        out.FragColor += 11;
    }
    int k = 0;
    for (; k < 20; k++)
    {
        out.FragColor += 12;
    }
    k += 3;
    out.FragColor += k;
    int l;
    if (k == 40)
    {
        l = 0;
        for (; l < 40; l++)
        {
            out.FragColor += 13;
        }
        return out;
    }
    else
    {
        l = k;
        out.FragColor += l;
    }
    int2 i_2 = int2(0);
    for (; i_2.x < 10; i_2.x += 4)
    {
        out.FragColor += i_2.y;
    }
    int o = k;
    for (int m = k; m < 40; m++)
    {
        out.FragColor += m;
    }
    out.FragColor += o;
    return out;
}

