#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(texture_buffer<float> uSamp [[texture(0)]], texture_buffer<float> uSampo [[texture(1)]])
{
    main0_out out = {};
    out.gl_Position = uSamp.read(uint(10)) + uSampo.read(uint(100));
    return out;
}

