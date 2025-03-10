#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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
    int _70 = int(gl_FragCoord.x) % 16;
    out._entryPointOutput = float4(dot(float3(_15.results[_70].a), _15.bar.xyz), _15.results[_70].b, 0.0, 0.0);
    return out;
}

