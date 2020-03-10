#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(uint gl_BaseVertex [[base_vertex]], uint gl_BaseInstance [[base_instance]])
{
    main0_out out = {};
    out.gl_Position = float4(float(int(gl_BaseVertex)), float(int(gl_BaseInstance)), 0.0, 1.0);
    return out;
}

