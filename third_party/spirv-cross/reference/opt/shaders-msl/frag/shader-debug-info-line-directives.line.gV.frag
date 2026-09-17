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
    bool _298 = in.iv.x < 0.0;
    if (_298)
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
    for (int _529 = 0; _529 < 4; _529++)
    {
#line 106 "test.frag"
        if (_298)
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
    for (int _533 = 0; _533 < 4; _533++)
    {
#line 106 "test.frag"
        if (_298)
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
        for (int _537 = 0; _537 < 4; _537++)
        {
#line 106 "test.frag"
            if (_298)
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

