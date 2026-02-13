#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    half C [[color(0)]];
    half D [[color(1)]];
};

struct main0_in
{
    half A [[user(locn0)]];
    half B [[user(locn1)]];
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
    out.C = clamp(half(fast::sinh(in.A)), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::cosh(in.A)), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::tanh(in.A)), in.A, in.B);
    out.D += out.C;
    out.C = clamp(asinh(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(acosh(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(atanh(in.A), in.A, in.B);
    out.D += out.C;
    out.C = clamp(half(fast::atan2(in.A, in.B)), in.A, in.B);
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
    return out;
}

