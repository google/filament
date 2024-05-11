#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float a_tmp [[function_constant(1)]];
constant float a = is_function_constant_defined(a_tmp) ? a_tmp : 1.0;
constant float b_tmp [[function_constant(2)]];
constant float b = is_function_constant_defined(b_tmp) ? b_tmp : 2.0;
constant int c_tmp [[function_constant(3)]];
constant int c = is_function_constant_defined(c_tmp) ? c_tmp : 3;
constant uint _18 = (uint(c) + 0u);
constant int _21 = (-c);
constant int _23 = (~c);
constant int d_tmp [[function_constant(4)]];
constant int d = is_function_constant_defined(d_tmp) ? d_tmp : 4;
constant int _26 = (c + d);
constant int _28 = (c - d);
constant int _30 = (c * d);
constant int _32 = (c / d);
constant uint e_tmp [[function_constant(5)]];
constant uint e = is_function_constant_defined(e_tmp) ? e_tmp : 5u;
constant uint f_tmp [[function_constant(6)]];
constant uint f = is_function_constant_defined(f_tmp) ? f_tmp : 6u;
constant uint _36 = (e / f);
constant int _38 = (c % d);
constant uint _40 = (e % f);
constant int _42 = (c >> d);
constant uint _44 = (e >> f);
constant int _46 = (c << d);
constant int _48 = (c | d);
constant int _50 = (c ^ d);
constant int _52 = (c & d);
constant bool g_tmp [[function_constant(7)]];
constant bool g = is_function_constant_defined(g_tmp) ? g_tmp : false;
constant bool h_tmp [[function_constant(8)]];
constant bool h = is_function_constant_defined(h_tmp) ? h_tmp : true;
constant bool _58 = (g || h);
constant bool _60 = (g && h);
constant bool _62 = (!g);
constant bool _64 = (g == h);
constant bool _66 = (g != h);
constant bool _68 = (c == d);
constant bool _70 = (c != d);
constant bool _72 = (c < d);
constant bool _74 = (e < f);
constant bool _76 = (c > d);
constant bool _78 = (e > f);
constant bool _80 = (c <= d);
constant bool _82 = (e <= f);
constant bool _84 = (c >= d);
constant bool _86 = (e >= f);
constant int _92 = int(e + 0u);
constant bool _94 = (c != int(0u));
constant bool _96 = (e != 0u);
constant int _100 = int(g);
constant uint _103 = uint(g);

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float t0 = a;
    float t1 = b;
    uint c0 = _18;
    int c1 = _21;
    int c2 = _23;
    int c3 = _26;
    int c4 = _28;
    int c5 = _30;
    int c6 = _32;
    uint c7 = _36;
    int c8 = _38;
    uint c9 = _40;
    int c10 = _42;
    uint c11 = _44;
    int c12 = _46;
    int c13 = _48;
    int c14 = _50;
    int c15 = _52;
    bool c16 = _58;
    bool c17 = _60;
    bool c18 = _62;
    bool c19 = _64;
    bool c20 = _66;
    bool c21 = _68;
    bool c22 = _70;
    bool c23 = _72;
    bool c24 = _74;
    bool c25 = _76;
    bool c26 = _78;
    bool c27 = _80;
    bool c28 = _82;
    bool c29 = _84;
    bool c30 = _86;
    int c31 = c8 + c3;
    int c32 = _92;
    bool c33 = _94;
    bool c34 = _96;
    int c35 = _100;
    uint c36 = _103;
    float c37 = float(g);
    out.FragColor = float4(t0 + t1);
    return out;
}

