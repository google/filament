#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct ins
{
    uchar2 m0;
    ushort2 m1;
};

struct main0_out
{
    uint4 f_out [[color(0)]];
};

struct main0_in
{
    uchar2 f_in_m0 [[user(locn0)]];
    ushort3 f_in_m1 [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    ins f_in = {};
    f_in.m0 = in.f_in_m0;
    f_in.m1 = in.f_in_m1.xy;
    out.f_out = uint4(uint2(f_in.m1), uint2(f_in.m0));
    return out;
}

