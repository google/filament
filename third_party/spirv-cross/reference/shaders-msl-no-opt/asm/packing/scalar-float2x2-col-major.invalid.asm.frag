#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Foo
{
    float4 a[1];
    char _m1_pad[8];
    float b;
};

struct main0_out
{
    float2 out_var_SV_Target [[color(0)]];
};

fragment main0_out main0(constant type_Foo& Foo [[buffer(0)]])
{
    main0_out out = {};
    out.out_var_SV_Target = (Foo.a[0u].xy + Foo.a[1u].xy) + float2(Foo.b);
    return out;
}

