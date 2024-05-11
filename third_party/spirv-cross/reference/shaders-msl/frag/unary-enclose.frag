#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vIn [[user(locn0)]];
    int4 vIn1 [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = -(-in.vIn);
    int4 a = ~(~in.vIn1);
    bool b = false;
    b = !(!b);
    return out;
}

