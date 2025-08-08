#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    os_log_default.log("Foo %f %f", 1.0, 2.0);
    float4 _16 = float4(0.0, 0.0, 0.0, 1.0);
    out.gl_Position = float4(0.0, 0.0, 0.0, 1.0);
    return out;
}

