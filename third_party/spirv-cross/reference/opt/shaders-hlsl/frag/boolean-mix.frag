static float2 FragColor;
static float2 x0;

struct SPIRV_Cross_Input
{
    float2 x0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float2 FragColor : SV_Target0;
};

void frag_main()
{
    bool2 _27 = (x0.x > x0.y).xx;
    FragColor = float2(_27.x ? float2(1.0f, 0.0f).x : float2(0.0f, 1.0f).x, _27.y ? float2(1.0f, 0.0f).y : float2(0.0f, 1.0f).y);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    x0 = stage_input.x0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
