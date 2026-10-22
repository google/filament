static float4 FragColor;
static int4 vA;
static int4 vB;

struct SPIRV_Cross_Input
{
    nointerpolation int4 vA : TEXCOORD0;
    nointerpolation int4 vB : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = float4(vA - vB * (vA / vB));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vA = stage_input.vA;
    vB = stage_input.vB;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
