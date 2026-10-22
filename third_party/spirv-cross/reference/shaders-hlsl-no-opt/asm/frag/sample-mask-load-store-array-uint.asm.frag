static uint gl_SampleMaskIn[1];
static uint gl_SampleMask[1];
struct SPIRV_Cross_Input
{
    uint gl_SampleMaskIn : SV_Coverage;
};

struct SPIRV_Cross_Output
{
    uint gl_SampleMask : SV_Coverage;
};

void frag_main()
{
    uint copy_sample_mask[1] = gl_SampleMaskIn;
    gl_SampleMask = copy_sample_mask;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_SampleMaskIn[0] = stage_input.gl_SampleMaskIn;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_SampleMask = gl_SampleMask[0];
    return stage_output;
}
