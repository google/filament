#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float2 FP32Out [[color(0)]];
    uint FP16Out [[color(1)]];
};

struct main0_in
{
    uint FP16 [[user(locn0)]];
    float2 FP32 [[user(locn1), flat]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FP32Out = float2(as_type<half2>(in.FP16));
    out.FP16Out = as_type<uint>(half2(in.FP32));
    return out;
}

