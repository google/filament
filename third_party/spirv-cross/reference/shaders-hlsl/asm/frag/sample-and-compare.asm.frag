Texture2D<float4> g_Texture : register(t0);
SamplerState g_Sampler : register(s0);
SamplerComparisonState g_CompareSampler : register(s1);

static float2 in_var_TEXCOORD0;
static float out_var_SV_Target;

struct SPIRV_Cross_Input
{
    float2 in_var_TEXCOORD0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float out_var_SV_Target : SV_Target0;
};

void frag_main()
{
    out_var_SV_Target = g_Texture.Sample(g_Sampler, in_var_TEXCOORD0).x + g_Texture.SampleCmpLevelZero(g_CompareSampler, in_var_TEXCOORD0, 0.5f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    in_var_TEXCOORD0 = stage_input.in_var_TEXCOORD0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.out_var_SV_Target = out_var_SV_Target;
    return stage_output;
}
