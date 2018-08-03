#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor_0 [[color(0)]];
    float4 FragColor_1 [[color(1)]];
    float4 FragColor_2 [[color(2)]];
    float4 FragColor_3 [[color(3)]];
};

struct main0_in
{
    float4 vA [[user(locn0)]];
    float4 vB [[user(locn1)]];
};

// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()
template<typename Tx, typename Ty>
Tx mod(Tx x, Ty y)
{
    return x - y * floor(x / y);
}

void write_deeper_in_function(thread float4 (&FragColor)[4], thread float4& vA, thread float4& vB)
{
    FragColor[3] = vA * vB;
}

void write_in_function(thread float4 (&FragColor)[4], thread float4& vA, thread float4& vB)
{
    FragColor[2] = vA - vB;
    write_deeper_in_function(FragColor, vA, vB);
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 FragColor[4] = {};
    FragColor[0] = mod(in.vA, in.vB);
    FragColor[1] = in.vA + in.vB;
    write_in_function(FragColor, in.vA, in.vB);
    out.FragColor_0 = FragColor[0];
    out.FragColor_1 = FragColor[1];
    out.FragColor_2 = FragColor[2];
    out.FragColor_3 = FragColor[3];
    return out;
}

