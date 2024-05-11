#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Foo
{
    float4 a[1];
    char _m1_pad[12];
    float b;
};

struct main0_out
{
    float2 out_var_SV_Target [[color(0)]];
};

fragment main0_out main0(constant type_Foo& Foo [[buffer(0)]])
{
    main0_out out = {};
    out.out_var_SV_Target = (float2(Foo.a[0][0u], Foo.a[1][0u]) + float2(Foo.a[0][1u], Foo.a[1][1u])) + float2(Foo.b);
    return out;
}

