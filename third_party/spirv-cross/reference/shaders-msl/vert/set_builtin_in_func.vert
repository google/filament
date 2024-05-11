#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
};

static inline __attribute__((always_inline))
void write_outblock(thread float4& gl_Position, thread float& gl_PointSize)
{
    gl_PointSize = 1.0;
    gl_Position = float4(gl_PointSize);
}

vertex main0_out main0()
{
    main0_out out = {};
    write_outblock(out.gl_Position, out.gl_PointSize);
    return out;
}

