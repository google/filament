#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 ov [[color(0)]];
};

struct main0_in
{
    float3 iv [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
#line 137 "test.frag"
#line 106 "test.frag"
    bool _288 = in.iv.x < 0.0;
    if (_288)
    {
#line 107 "test.frag"
        out.ov.x = 50.0;
    }
    else
    {
#line 109 "test.frag"
        out.ov.x = 60.0;
    }
#line 114 "test.frag"
    for (int _519 = 0; _519 < 4; _519++)
    {
#line 106 "test.frag"
        if (_288)
        {
#line 107 "test.frag"
            out.ov.x = 50.0;
        }
        else
        {
#line 109 "test.frag"
            out.ov.x = 60.0;
        }
#line 117 "test.frag"
        if (in.iv.y < 0.0)
        {
#line 118 "test.frag"
            out.ov.y = 70.0;
        }
        else
        {
#line 120 "test.frag"
            out.ov.y = 80.0;
        }
    }
#line 126 "test.frag"
    for (int _523 = 0; _523 < 4; _523++)
    {
#line 106 "test.frag"
        if (_288)
        {
#line 107 "test.frag"
            out.ov.x = 50.0;
        }
        else
        {
#line 109 "test.frag"
            out.ov.x = 60.0;
        }
#line 114 "test.frag"
        for (int _527 = 0; _527 < 4; _527++)
        {
#line 106 "test.frag"
            if (_288)
            {
#line 107 "test.frag"
                out.ov.x = 50.0;
            }
            else
            {
#line 109 "test.frag"
                out.ov.x = 60.0;
            }
#line 117 "test.frag"
            if (in.iv.y < 0.0)
            {
#line 118 "test.frag"
                out.ov.y = 70.0;
            }
            else
            {
#line 120 "test.frag"
                out.ov.y = 80.0;
            }
        }
#line 130 "test.frag"
        if (in.iv.z < 0.0)
        {
#line 131 "test.frag"
            out.ov.z = 100.0;
        }
        else
        {
#line 133 "test.frag"
            out.ov.z = 120.0;
        }
    }
#line 142 "test.frag"
    return out;
}

