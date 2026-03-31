#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    half C [[color(0)]];
    half D [[color(1)]];
    half3 o3 [[color(2)]];
};

struct main0_in
{
    half A [[user(locn0)]];
    half B [[user(locn1)]];
    half3 v3 [[user(locn2)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.D = half(0.0);
    out.C = clamp(sin(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(cos(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(tan(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(asin(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(acos(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(atan(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::sinh(float(in.A))), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::cosh(float(in.A))), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::tanh(float(in.A))), in.A, in.B);
    out.D += out.C;
    out.C = clamp(asinh(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(acosh(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(atanh(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::atan2(float(in.A), float(in.B))), in.A, in.B);
    out.D += out.C;
    out.C = clamp(powr(in.A, in.B), in.A, in.B);
    out.D += out.C;
    out.C = clamp(exp(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(exp2(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(log(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(log2(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(sqrt(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(rsqrt(in.A), in.A, in.B);
    out.D += out.C;
    out.o3 = half3(fast::sinh(float3(in.v3)));
    out.o3 += half3(fast::cosh(float3(in.v3)));
    out.o3 += half3(fast::tanh(float3(in.v3)));
    out.o3 += sin(in.v3);
    out.o3 += cos(in.v3);
    out.o3 += tan(in.v3);
    out.o3 += asin(in.v3);
    out.o3 += acos(in.v3);
    out.o3 += atan(in.v3);
    out.o3 += asinh(in.v3);
    out.o3 += acosh(in.v3);
    out.o3 += atanh(in.v3);
    out.o3 += half3(fast::atan2(float3(out.o3), float3(in.v3)));
    out.o3 += powr(out.o3, in.v3);
    out.o3 += exp(in.v3);
    out.o3 += exp2(in.v3);
    out.o3 += log(in.v3);
    out.o3 += log2(in.v3);
    out.o3 += sqrt(in.v3);
    out.o3 += rsqrt(in.v3);
    return out;
}

