struct Bar
{
    float v[2];
    float w;
};

struct V
{
    float a : TEXCOORD0;
    float b[2] : TEXCOORD1;
    Bar c[2] : TEXCOORD3;
    Bar d : TEXCOORD9;
};

static V _14;

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

void main(out V stage_output_14)
{
    vert_main();
    stage_output_14 = _14;
}
