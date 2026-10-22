#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant uint* spvDrawIndex [[buffer(19)]], uint gl_BaseVertex [[base_vertex]], uint gl_BaseInstance [[base_instance]])
{
    main0_out out = {};
    uint gl_DrawID = *spvDrawIndex;
    out.gl_Position = float4(float(int(gl_BaseVertex)), float(int(gl_BaseInstance)), float(int(gl_DrawID)), 1.0);
    return out;
}

