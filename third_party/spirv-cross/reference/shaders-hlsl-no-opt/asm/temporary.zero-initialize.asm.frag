static float4 FragColor;
static int vA;
static int vB;

struct SPIRV_Cross_Input
{
    nointerpolation int vA : TEXCOORD0;
    nointerpolation int vB : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = 0.0f.xxxx;
    int _10 = 0;
    int _15 = 0;
    for (int _16 = 0, _17 = 0; _16 < vA; _17 = _15, _16 += _10)
    {
        if ((vA + _16) == 20)
        {
            _15 = 50;
        }
        else
        {
            _15 = ((vB + _16) == 40) ? 60 : _17;
        }
        _10 = _15 + 10;
        FragColor += 1.0f.xxxx;
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
