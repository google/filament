static int gl_SampleID;
static int gl_SampleMaskIn;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    uint gl_SampleID : SV_SampleIndex;
    uint gl_SampleMaskIn : SV_Coverage;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    if ((gl_SampleMaskIn & (1 << gl_SampleID)) != 0)
    {
        FragColor = 1.0f.xxxx;
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_SampleID = stage_input.gl_SampleID;
    gl_SampleMaskIn = stage_input.gl_SampleMaskIn;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
