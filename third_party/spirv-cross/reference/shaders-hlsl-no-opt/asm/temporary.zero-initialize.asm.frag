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
    int _49 = 0;
    int _58 = 0;
    for (int _57 = 0, _60 = 0; _57 < vA; _60 = _58, _57 += _49)
    {
        if ((vA + _57) == 20)
        {
            _58 = 50;
        }
        else
        {
            _58 = ((vB + _57) == 40) ? 60 : _60;
        }
        _49 = _58 + 10;
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
