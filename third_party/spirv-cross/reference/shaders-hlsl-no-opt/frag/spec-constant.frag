#ifndef SPIRV_CROSS_CONSTANT_ID_1
#define SPIRV_CROSS_CONSTANT_ID_1 1.0f
#endif
static const float a = SPIRV_CROSS_CONSTANT_ID_1;
#ifndef SPIRV_CROSS_CONSTANT_ID_2
#define SPIRV_CROSS_CONSTANT_ID_2 2.0f
#endif
static const float b = SPIRV_CROSS_CONSTANT_ID_2;
#ifndef SPIRV_CROSS_CONSTANT_ID_3
#define SPIRV_CROSS_CONSTANT_ID_3 3
#endif
static const int c = SPIRV_CROSS_CONSTANT_ID_3;
static const uint _18 = (uint(c) + 0u);
static const int _21 = (-c);
static const int _23 = (~c);
#ifndef SPIRV_CROSS_CONSTANT_ID_4
#define SPIRV_CROSS_CONSTANT_ID_4 4
#endif
static const int d = SPIRV_CROSS_CONSTANT_ID_4;
static const int _26 = (c + d);
static const int _28 = (c - d);
static const int _30 = (c * d);
static const int _32 = (c / d);
#ifndef SPIRV_CROSS_CONSTANT_ID_5
#define SPIRV_CROSS_CONSTANT_ID_5 5u
#endif
static const uint e = SPIRV_CROSS_CONSTANT_ID_5;
#ifndef SPIRV_CROSS_CONSTANT_ID_6
#define SPIRV_CROSS_CONSTANT_ID_6 6u
#endif
static const uint f = SPIRV_CROSS_CONSTANT_ID_6;
static const uint _36 = (e / f);
static const int _38 = (c % d);
static const uint _40 = (e % f);
static const int _42 = (c >> d);
static const uint _44 = (e >> f);
static const int _46 = (c << d);
static const int _48 = (c | d);
static const int _50 = (c ^ d);
static const int _52 = (c & d);
#ifndef SPIRV_CROSS_CONSTANT_ID_7
#define SPIRV_CROSS_CONSTANT_ID_7 false
#endif
static const bool g = SPIRV_CROSS_CONSTANT_ID_7;
#ifndef SPIRV_CROSS_CONSTANT_ID_8
#define SPIRV_CROSS_CONSTANT_ID_8 true
#endif
static const bool h = SPIRV_CROSS_CONSTANT_ID_8;
static const bool _58 = (g || h);
static const bool _60 = (g && h);
static const bool _62 = (!g);
static const bool _64 = (g == h);
static const bool _66 = (g != h);
static const bool _68 = (c == d);
static const bool _70 = (c != d);
static const bool _72 = (c < d);
static const bool _74 = (e < f);
static const bool _76 = (c > d);
static const bool _78 = (e > f);
static const bool _80 = (c <= d);
static const bool _82 = (e <= f);
static const bool _84 = (c >= d);
static const bool _86 = (e >= f);
static const int _92 = int(e + 0u);
static const bool _94 = (c != int(0u));
static const bool _96 = (e != 0u);
static const int _100 = int(g);
static const uint _103 = uint(g);
static const int _111 = (c + 3);
static const int _118 = (c + 2);
static const int _124 = (d + 2);

struct Foo
{
    float elems[_124];
};

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
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
    float vec0[_111][8];
    vec0[0][0] = 10.0f;
    float vec1[_118];
    vec1[0] = 20.0f;
    Foo foo;
    foo.elems[c] = 10.0f;
    FragColor = (((t0 + t1).xxxx + vec0[0][0].xxxx) + vec1[0].xxxx) + foo.elems[c].xxxx;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
