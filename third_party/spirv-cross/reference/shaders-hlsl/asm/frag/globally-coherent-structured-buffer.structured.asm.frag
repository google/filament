globallycoherent RWStructuredBuffer<float4> TestBuffer : register(u0);

static float4 out_var_SV_Target0;

struct SPIRV_Cross_Output
{
    float4 out_var_SV_Target0 : SV_Target0;
};

void frag_main()
{
    out_var_SV_Target0 = TestBuffer[0u];
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.out_var_SV_Target0 = out_var_SV_Target0;
    return stage_output;
}
