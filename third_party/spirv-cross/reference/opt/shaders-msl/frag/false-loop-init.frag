#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 result [[color(0)]];
};

struct main0_in
{
    float4 accum [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.result = float4(0.0);
    for (int _48 = 0; _48 < 4; )
    {
        out.result += in.accum;
        _48 += int((in.accum.y > 10.0) ? 40u : 30u);
        continue;
    }
    return out;
}

