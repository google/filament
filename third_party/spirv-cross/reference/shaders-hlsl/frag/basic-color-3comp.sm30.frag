static float3 FragColor;
static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : COLOR0;
};

void frag_main()
{
    FragColor = vColor.xyz;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = float4(FragColor, 0.0);
    return stage_output;
}
