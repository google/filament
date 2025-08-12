#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct ResType
{
    float4 _m0;
    float4 _m1;
};

struct main0_out
{
    float4 vo0 [[color(0)]];
    float4 vo1 [[color(1)]];
};

struct main0_in
{
    float4 v [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 modf_inner(thread float4& v, thread float4& vo1)
{
    ResType _17;
    _17._m0 = modf(v, _17._m1);
    vo1 = _17._m1;
    return _17._m0;
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 _23 = modf_inner(in.v, out.vo1);
    out.vo0 = _23;
    return out;
}

