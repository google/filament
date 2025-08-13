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

struct Foo
{
    packed_float3 a;
    float b;
};

struct buf
{
    Foo results[16];
    float4 bar;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0(constant buf& _15 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    int _70 = spvSMod(int(gl_FragCoord.x), 16);
    out._entryPointOutput = float4(dot(float3(_15.results[_70].a), _15.bar.xyz), _15.results[_70].b, 0.0, 0.0);
    return out;
}

