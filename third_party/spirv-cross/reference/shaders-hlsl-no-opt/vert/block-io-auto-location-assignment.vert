struct Bar
{
    float v[2];
    float w;
};

struct V
{
    float a;
    float b[2];
    Bar c[2];
    Bar d;
};

static V _14;

struct SPIRV_Cross_Output
{
    float V_a : TEXCOORD0;
    float V_b[2] : TEXCOORD1;
    Bar V_c[2] : TEXCOORD3;
    Bar V_d : TEXCOORD9;
};

void vert_main()
{
    _14.a = 1.0f;
    _14.b[0] = 2.0f;
    _14.b[1] = 3.0f;
    _14.c[0].v[0] = 4.0f;
    _14.c[0].v[1] = 5.0f;
    _14.c[0].w = 6.0f;
    _14.c[1].v[0] = 7.0f;
    _14.c[1].v[1] = 8.0f;
    _14.c[1].w = 9.0f;
    _14.d.v[0] = 10.0f;
    _14.d.v[1] = 11.0f;
    _14.d.w = 12.0f;
}

SPIRV_Cross_Output main()
{
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.V_a = _14.a;
    stage_output.V_b = _14.b;
    stage_output.V_c = _14.c;
    stage_output.V_d = _14.d;
    return stage_output;
}
