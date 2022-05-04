static float4 FragColor;
static float4 vIn;

struct SPIRV_Cross_Input
{
    float4 vIn : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = vIn;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIn = stage_input.vIn;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
