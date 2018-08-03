#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4 Data[3][5];
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    int2 aIndex [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _20 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _20.Data[in.aIndex.x][in.aIndex.y];
    return out;
}

