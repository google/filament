struct Foo
{
    float a;
    float b;
};

static const float _16[4] = { 1.0f, 4.0f, 3.0f, 2.0f };
static const Foo _24 = { 10.0f, 20.0f };
static const Foo _27 = { 30.0f, 40.0f };
static const Foo _28[2] = { { 10.0f, 20.0f }, { 30.0f, 40.0f } };

static float4 FragColor;
static int _line;

struct SPIRV_Cross_Input
{
    nointerpolation int _line : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = _16[_line].xxxx;
    FragColor += (_28[_line].a * _28[1 - _line].a).xxxx;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    _line = stage_input._line;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
