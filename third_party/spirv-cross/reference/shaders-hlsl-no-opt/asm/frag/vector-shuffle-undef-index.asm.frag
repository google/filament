static float4 undef;

static float4 FragColor;
static float4 vFloat;

struct SPIRV_Cross_Input
{
    float4 vFloat : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = float4(undef.x, vFloat.y, 0.0f, vFloat.w) + float4(vFloat.z, vFloat.y, 0.0f, vFloat.w);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vFloat = stage_input.vFloat;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
