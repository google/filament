#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

[[ patch(triangle, 0) ]] vertex main0_out main0()
{
    main0_out out = {};
    out.gl_Position = float4(1.0);
    return out;
}

