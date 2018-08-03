static float4 FragColor;
static float4x4 m;

struct SPIRV_Cross_Input
{
    float4x4 m : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = ((m[0] + m[1]) + m[2]) + m[3];
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    m = stage_input.m;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
