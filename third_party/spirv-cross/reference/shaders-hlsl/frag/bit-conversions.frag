static float2 value;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float2 value : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    int i = asint(value.x);
    FragColor = float4(1.0f, 0.0f, asfloat(i), 1.0f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    value = stage_input.value;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
