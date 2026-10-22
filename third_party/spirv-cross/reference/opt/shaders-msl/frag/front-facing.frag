#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vA [[user(locn0)]];
    float4 vB [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], bool gl_FrontFacing [[front_facing]])
{
    main0_out out = {};
    if (gl_FrontFacing)
    {
        out.FragColor = in.vA;
    }
    else
    {
        out.FragColor = in.vB;
    }
    return out;
}

