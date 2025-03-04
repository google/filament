static int gl_SampleMaskIn[1];
static int gl_SampleMask[1];
static float4 FragColor;

struct SPIRV_Cross_Input
{
    uint gl_SampleMaskIn : SV_Coverage;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
    uint gl_SampleMask : SV_Coverage;
};

void frag_main()
{
    FragColor = 1.0f.xxxx;
    gl_SampleMask[0] = gl_SampleMaskIn[0];
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_SampleMaskIn[0] = stage_input.gl_SampleMaskIn;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_SampleMask = gl_SampleMask[0];
    stage_output.FragColor = FragColor;
    return stage_output;
}
