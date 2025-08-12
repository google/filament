#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Implementation of signed integer mod accurate to SPIR-V specification
template<typename Tx, typename Ty>
inline Tx spvSMod(Tx x, Ty y)
{
    Tx remainder = x - y * (x / y);
    return select(Tx(remainder + y), remainder, remainder == 0 || (x >= 0) == (y >= 0));
}

struct UBO
{
    float4 results[1024];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int vIn [[user(locn0)]];
    int vIn2 [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], device UBO& _34 [[buffer(0)]], texture2d<int> Buf [[texture(0)]], sampler BufSmplr [[sampler(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    int4 coords = Buf.read(uint2(int2(gl_FragCoord.xy)), 0);
    float4 foo = _34.results[spvSMod(coords.x, 16)];
    int c = in.vIn * in.vIn;
    int d = in.vIn2 * in.vIn2;
    out.FragColor = (foo + foo) + _34.results[c + d];
    return out;
}

