#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#ifndef SPIRV_CROSS_CONSTANT_ID_1
#define SPIRV_CROSS_CONSTANT_ID_1 1.0
#endif
constant float a = SPIRV_CROSS_CONSTANT_ID_1;
#ifndef SPIRV_CROSS_CONSTANT_ID_2
#define SPIRV_CROSS_CONSTANT_ID_2 2.0
#endif
constant float b = SPIRV_CROSS_CONSTANT_ID_2;
#ifndef SPIRV_CROSS_CONSTANT_ID_3
#define SPIRV_CROSS_CONSTANT_ID_3 3
#endif
constant int c = SPIRV_CROSS_CONSTANT_ID_3;
constant uint _18 = (uint(c) + 0u);
constant int _21 = (-c);
constant int _23 = (~c);
#ifndef SPIRV_CROSS_CONSTANT_ID_4
#define SPIRV_CROSS_CONSTANT_ID_4 4
#endif
constant int d = SPIRV_CROSS_CONSTANT_ID_4;
constant int _26 = (c + d);
constant int _28 = (c - d);
constant int _30 = (c * d);
constant int _32 = (c / d);
#ifndef SPIRV_CROSS_CONSTANT_ID_5
#define SPIRV_CROSS_CONSTANT_ID_5 5u
#endif
constant uint e = SPIRV_CROSS_CONSTANT_ID_5;
#ifndef SPIRV_CROSS_CONSTANT_ID_6
#define SPIRV_CROSS_CONSTANT_ID_6 6u
#endif
constant uint f = SPIRV_CROSS_CONSTANT_ID_6;
constant uint _36 = (e / f);
constant int _38 = (c % d);
constant uint _40 = (e % f);
constant int _42 = (c >> d);
constant uint _44 = (e >> f);
constant int _46 = (c << d);
constant int _48 = (c | d);
constant int _50 = (c ^ d);
constant int _52 = (c & d);
#ifndef SPIRV_CROSS_CONSTANT_ID_7
#define SPIRV_CROSS_CONSTANT_ID_7 false
#endif
constant bool g = SPIRV_CROSS_CONSTANT_ID_7;
#ifndef SPIRV_CROSS_CONSTANT_ID_8
#define SPIRV_CROSS_CONSTANT_ID_8 true
#endif
constant bool h = SPIRV_CROSS_CONSTANT_ID_8;
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

