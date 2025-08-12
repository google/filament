#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct buff_t
{
    int m0[1024];
};

struct main0_out
{
    float4 frag_clr [[color(0)]];
};

fragment main0_out main0(device buff_t& buff [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    int4 _16 = int4(gl_FragCoord);
    out.frag_clr = float4(0.0, 0.0, 1.0, 1.0);
    buff.m0[(_16.y * 32) + _16.x] = 1;
    discard_fragment();
    return out;
}

