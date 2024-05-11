#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Foo
{
    float2x4 a;
    char _m1_pad[8];
    float b;
};

struct main0_out
{
    float3 out_var_SV_Target [[color(0)]];
};

fragment main0_out main0(constant type_Foo& Foo [[buffer(0)]])
{
    main0_out out = {};
    out.out_var_SV_Target = (float3(Foo.a[0][0u], Foo.a[1][0u], Foo.a[2][0u]) + float3(Foo.a[0][1u], Foo.a[1][1u], Foo.a[2][1u])) + float3(Foo.b);
    return out;
}

