#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant bool _3_tmp [[function_constant(0)]];
constant bool _3 = is_function_constant_defined(_3_tmp) ? _3_tmp : true;
constant uint _4 = is_function_constant_defined(_3_tmp) ? uint(_3_tmp) : 0u;

struct main0_out
{
    float4 m_2 [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.m_2 = float4(float(_4) + float(_3));
    return out;
}

