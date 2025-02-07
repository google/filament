#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float vColor [[user(locn0)]];
};

#line 8 "test.frag"
fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
#line 8 "test.frag"
    out.FragColor = 1.0;
#line 9 "test.frag"
    out.FragColor = 2.0;
#line 10 "test.frag"
    if (in.vColor < 0.0)
    {
#line 12 "test.frag"
        out.FragColor = 3.0;
    }
    else
    {
#line 16 "test.frag"
        out.FragColor = 4.0;
    }
#line 19 "test.frag"
    for (int _131 = 0; float(_131) < (40.0 + in.vColor); )
    {
#line 21 "test.frag"
        out.FragColor += 0.20000000298023223876953125;
#line 22 "test.frag"
        out.FragColor += 0.300000011920928955078125;
#line 19 "test.frag"
        _131 += (int(in.vColor) + 5);
        continue;
    }
#line 25 "test.frag"
    switch (int(in.vColor))
    {
        case 0:
        {
#line 28 "test.frag"
            out.FragColor += 0.20000000298023223876953125;
#line 29 "test.frag"
            break;
        }
        case 1:
        {
#line 32 "test.frag"
            out.FragColor += 0.4000000059604644775390625;
#line 33 "test.frag"
            break;
        }
        default:
        {
#line 36 "test.frag"
            out.FragColor += 0.800000011920928955078125;
#line 37 "test.frag"
            break;
        }
    }
    for (;;)
    {
#line 42 "test.frag"
        out.FragColor += (10.0 + in.vColor);
#line 43 "test.frag"
        if (out.FragColor < 100.0)
        {
        }
        else
        {
            break;
        }
    }
#line 48 "test.frag"
    return out;
}

