#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct AStruct
{
    float4 foobar;
};

struct main0_out
{
    float3 FragColor [[color(0)]];
};

void someFunction(thread AStruct& s)
{
    s.foobar = float4(1.0);
}

void otherFunction(thread float3& global_variable)
{
    global_variable = float3(1.0);
}

fragment main0_out main0()
{
    main0_out out = {};
    AStruct param;
    someFunction(param);
    AStruct inputs = param;
    float3 global_variable;
    otherFunction(global_variable);
    out.FragColor = global_variable;
    return out;
}

