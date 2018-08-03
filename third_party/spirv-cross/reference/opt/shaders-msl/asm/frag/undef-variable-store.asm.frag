#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out._entryPointOutput = float4(1.0, 1.0, 0.0, 1.0);
    return out;
}

