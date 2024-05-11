static bool gl_FrontFacing;
static float4 FragColor;
static float4 vA;
static float4 vB;

struct SPIRV_Cross_Input
{
    float4 vA : TEXCOORD0;
    float4 vB : TEXCOORD1;
    bool gl_FrontFacing : SV_IsFrontFace;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    if (gl_FrontFacing)
    {
        FragColor = vA;
    }
    else
    {
        FragColor = vB;
    }
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FrontFacing = stage_input.gl_FrontFacing;
    vA = stage_input.vA;
    vB = stage_input.vB;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
