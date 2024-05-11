static float2 interpolant;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float2 interpolant : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = float4(0.0f, 0.0f, 0.0f, EvaluateAttributeSnapped(interpolant, 0.100000001490116119384765625f.xx).x) + float4(0.0f, 0.0f, 0.0f, ddx_coarse(interpolant.x));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    interpolant = stage_input.interpolant;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
