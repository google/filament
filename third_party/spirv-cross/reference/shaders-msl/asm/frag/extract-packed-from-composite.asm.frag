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
    float3 a;
    float b;
};

struct Foo_1
{
    packed_float3 a;
    float b;
};

struct buf
{
    Foo_1 results[16];
    float4 bar;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

static inline __attribute__((always_inline))
float4 _main(thread const float4& pos, constant buf& _15)
{
    int _32 = spvSMod(int(pos.x), 16);
    Foo foo;
    foo.a = float3(_15.results[_32].a);
    foo.b = _15.results[_32].b;
    return float4(dot(foo.a, _15.bar.xyz), foo.b, 0.0, 0.0);
}

fragment main0_out main0(constant buf& _15 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float4 pos = gl_FragCoord;
    float4 param = pos;
    out._entryPointOutput = _main(param, _15);
    return out;
}

