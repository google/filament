#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int vIndex [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    int i = 0;
    main0_out out = {};
    int j;
    int _33;
    int _34;
    if (in.vIndex != 0 && in.vIndex != 1 && in.vIndex != 11 && in.vIndex != 2 && in.vIndex != 3 && in.vIndex != 4 && in.vIndex != 5)
    {
        _33 = 2;
    }
    if (in.vIndex == 1 || in.vIndex == 11)
    {
        _34 = 1;
    }
    switch (in.vIndex)
    {
        case 0:
        {
            _33 = 3;
        }
        default:
        {
            j = _33;
            _34 = 0;
        }
        case 1:
        case 11:
        {
            j = _34;
        }
        case 2:
        {
            break;
        }
        case 3:
        {
            if (in.vIndex > 3)
            {
                i = 0;
                break;
            }
            else
            {
                break;
            }
        }
        case 4:
        {
        }
        case 5:
        {
            i = 0;
            break;
        }
    }
    out.FragColor = float4(float(i));
    return out;
}

