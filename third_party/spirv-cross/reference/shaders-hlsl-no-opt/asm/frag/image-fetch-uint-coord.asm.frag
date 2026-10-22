Texture2D<float4> Tex : register(t0);

static uint3 in_var_TEXCOORD0;
static float4 out_var_SV_Target0;

struct SPIRV_Cross_Input
{
    nointerpolation uint3 in_var_TEXCOORD0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 out_var_SV_Target0 : SV_Target0;
};

void frag_main()
{
    out_var_SV_Target0 = Tex.Load(int3(in_var_TEXCOORD0.xy, in_var_TEXCOORD0.z));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    in_var_TEXCOORD0 = stage_input.in_var_TEXCOORD0;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.out_var_SV_Target0 = out_var_SV_Target0;
    return stage_output;
}
