#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_Foo
{
    packed_float3 a[1];
    float b;
};

struct main0_out
{
    float3 out_var_SV_Target [[color(0)]];
};

fragment main0_out main0(constant type_Foo& Foo [[buffer(0)]])
{
    main0_out out = {};
    out.out_var_SV_Target = float3(Foo.a[0]) + float3(Foo.b);
    return out;
}

