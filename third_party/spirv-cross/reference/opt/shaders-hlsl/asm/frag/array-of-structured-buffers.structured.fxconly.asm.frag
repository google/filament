struct Data
{
    float4 Color;
};

StructuredBuffer<Data> Colors[2] : register(t0);

static float4 out_var_SV_Target;

struct SPIRV_Cross_Output
{
    float4 out_var_SV_Target : SV_Target0;
};

void frag_main()
{
    out_var_SV_Target = Colors[1][3u].Color;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.out_var_SV_Target = out_var_SV_Target;
    return stage_output;
}
