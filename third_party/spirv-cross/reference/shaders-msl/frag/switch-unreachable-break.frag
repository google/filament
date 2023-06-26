#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    int cond;
    int cond2;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBO& _15 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = float4(10.0);
    switch (_15.cond)
    {
        case 1:
        {
            if (_15.cond2 < 50)
            {
                break;
            }
            else
            {
                discard_fragment();
            }
            break; // unreachable workaround
        }
        default:
        {
            out.FragColor = float4(20.0);
            break;
        }
    }
    return out;
}

