#version 310 es
precision mediump float;
precision highp int;

#ifndef SPIRV_CROSS_CONSTANT_ID_1
#define SPIRV_CROSS_CONSTANT_ID_1 1.0
#endif
const float a = SPIRV_CROSS_CONSTANT_ID_1;
#ifndef SPIRV_CROSS_CONSTANT_ID_2
#define SPIRV_CROSS_CONSTANT_ID_2 2.0
#endif
const float b = SPIRV_CROSS_CONSTANT_ID_2;
#ifndef SPIRV_CROSS_CONSTANT_ID_3
#define SPIRV_CROSS_CONSTANT_ID_3 3
#endif
const int c = SPIRV_CROSS_CONSTANT_ID_3;
const uint _18 = (uint(c) + 0u);
const int _21 = (-c);
const int _23 = (~c);
#ifndef SPIRV_CROSS_CONSTANT_ID_4
#define SPIRV_CROSS_CONSTANT_ID_4 4
#endif
const int d = SPIRV_CROSS_CONSTANT_ID_4;
const int _26 = (c + d);
const int _28 = (c - d);
const int _30 = (c * d);
const int _32 = (c / d);
#ifndef SPIRV_CROSS_CONSTANT_ID_5
#define SPIRV_CROSS_CONSTANT_ID_5 5u
#endif
const uint e = SPIRV_CROSS_CONSTANT_ID_5;
#ifndef SPIRV_CROSS_CONSTANT_ID_6
#define SPIRV_CROSS_CONSTANT_ID_6 6u
#endif
const uint f = SPIRV_CROSS_CONSTANT_ID_6;
const uint _36 = (e / f);
const int _38 = (c % d);
const uint _40 = (e % f);
const int _42 = (c >> d);
const uint _44 = (e >> f);
const int _46 = (c << d);
const int _48 = (c | d);
const int _50 = (c ^ d);
const int _52 = (c & d);
#ifndef SPIRV_CROSS_CONSTANT_ID_7
#define SPIRV_CROSS_CONSTANT_ID_7 false
#endif
const bool g = SPIRV_CROSS_CONSTANT_ID_7;
#ifndef SPIRV_CROSS_CONSTANT_ID_8
#define SPIRV_CROSS_CONSTANT_ID_8 true
#endif
const bool h = SPIRV_CROSS_CONSTANT_ID_8;
const bool _58 = (g || h);
const bool _60 = (g && h);
const bool _62 = (!g);
const bool _64 = (g == h);
const bool _66 = (g != h);
const bool _68 = (c == d);
const bool _70 = (c != d);
const bool _72 = (c < d);
const bool _74 = (e < f);
const bool _76 = (c > d);
const bool _78 = (e > f);
const bool _80 = (c <= d);
const bool _82 = (e <= f);
const bool _84 = (c >= d);
const bool _86 = (e >= f);
const int _92 = int(e + 0u);
const bool _94 = (c != int(0u));
const bool _96 = (e != 0u);
const int _100 = int(g);
const uint _103 = uint(g);
const int _118 = (c + 3);
const int _127 = (c + 2);
const int _135 = (d + 2);

struct Foo
{
    float elems[_135];
};

layout(location = 0) out vec4 FragColor;

void main()
{
    float t0 = a;
    float t1 = b;
    mediump uint c0 = _18;
    mediump int c1 = _21;
    mediump int c2 = _23;
    mediump int c3 = _26;
    mediump int c4 = _28;
    mediump int c5 = _30;
    mediump int c6 = _32;
    mediump uint c7 = _36;
    mediump int c8 = _38;
    mediump uint c9 = _40;
    mediump int c10 = _42;
    mediump uint c11 = _44;
    mediump int c12 = _46;
    mediump int c13 = _48;
    mediump int c14 = _50;
    mediump int c15 = _52;
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
    mediump int c31 = c8 + c3;
    mediump int c32 = _92;
    bool c33 = _94;
    bool c34 = _96;
    mediump int c35 = _100;
    mediump uint c36 = _103;
    float c37 = float(g);
    float vec0[_118][8];
    float vec1[_127];
    Foo foo;
    FragColor = ((vec4(t0 + t1) + vec4(vec0[0][0])) + vec4(vec1[0])) + vec4(foo.elems[c]);
}

