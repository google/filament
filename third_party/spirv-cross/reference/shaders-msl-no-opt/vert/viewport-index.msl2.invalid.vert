#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
    uint gl_ViewportIndex [[viewport_array_index]];
};

struct main0_in
{
    float4 coord [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.coord;
    out.gl_ViewportIndex = uint(int(in.coord.z));
    return out;
}

