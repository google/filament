static int gl_SampleMask;
static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
    uint gl_SampleMask : SV_Coverage;
};

void frag_main()
{
    FragColor = 1.0f.xxxx;
    gl_SampleMask = 0;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_SampleMask = gl_SampleMask;
    stage_output.FragColor = FragColor;
    return stage_output;
}
