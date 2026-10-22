static const float _17[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

static float4 FragColor;
static float4 v0;

struct SPIRV_Cross_Input
{
    float4 v0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    for (int i = 0; i < 4; i++, FragColor += _17[i].xxxx)
    {
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    v0 = stage_input.v0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
