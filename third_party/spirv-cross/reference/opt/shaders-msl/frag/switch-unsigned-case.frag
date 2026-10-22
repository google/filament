#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Buff
{
    uint TestVal;
};

struct main0_out
{
    float4 fsout_Color [[color(0)]];
};

fragment main0_out main0(constant Buff& _15 [[buffer(0)]])
{
    main0_out out = {};
    out.fsout_Color = float4(1.0);
    switch (_15.TestVal)
    {
        case 0u:
        {
            out.fsout_Color = float4(0.100000001490116119384765625);
            break;
        }
        case 1u:
        {
            out.fsout_Color = float4(0.20000000298023223876953125);
            break;
        }
    }
    return out;
}

