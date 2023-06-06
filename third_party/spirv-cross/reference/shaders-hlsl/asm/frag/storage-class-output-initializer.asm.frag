static const float4 _20[2] = { float4(1.0f, 2.0f, 3.0f, 4.0f), 10.0f.xxxx };

static float4 FragColors[2] = _20;
static float4 FragColor = 5.0f.xxxx;

struct SPIRV_Cross_Output
{
    float4 FragColors[2] : SV_Target0;
    float4 FragColor : SV_Target2;
};

void frag_main()
{
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColors = FragColors;
    stage_output.FragColor = FragColor;
    return stage_output;
}
