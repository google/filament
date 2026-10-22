#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template <typename F> struct SpvHalfTypeSelector;
template <> struct SpvHalfTypeSelector<float> { public: using H = half; };
template<uint N> struct SpvHalfTypeSelector<vec<float, N>> { using H = vec<half, N>; };
template<typename F, typename H = typename SpvHalfTypeSelector<F>::H>
[[clang::optnone]] F spvQuantizeToF16(F fval)
{
    H hval = H(fval);
    hval = select(copysign(H(0), hval), hval, isnormal(hval) || isinf(hval) || isnan(hval));
    return F(hval);
}

constant int _13_tmp [[function_constant(201)]];
constant int _13 = is_function_constant_defined(_13_tmp) ? _13_tmp : -10;
constant int _15 = (_13 + 2);
constant uint _24_tmp [[function_constant(202)]];
constant uint _24 = is_function_constant_defined(_24_tmp) ? _24_tmp : 100u;
constant uint _26 = (_24 % 5u);
constant int _61 = _13 - (-3) * (_13 / (-3));
constant int4 _36 = int4(20, 30, _15, _61);
constant int2 _41 = int2(_36.y, _36.x);
constant int _62 = _36.y;
constant float _57_tmp [[function_constant(200)]];
constant float _57 = is_function_constant_defined(_57_tmp) ? _57_tmp : 3.141590118408203125;
constant float _63 = spvQuantizeToF16(_57);

struct main0_out
{
    int m_58 [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float4 pos = float4(0.0);
    pos.y += float(_15);
    pos.z += float(_26);
    pos += float4(_36);
    float2 _46 = pos.xy + float2(_41);
    pos = float4(_46.x, _46.y, pos.z, pos.w);
    out.gl_Position = pos;
    out.m_58 = _62;
    return out;
}

