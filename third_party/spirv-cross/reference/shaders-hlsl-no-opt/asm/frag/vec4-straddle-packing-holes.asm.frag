cbuffer type_Test : register(b0)
{
    float3 Test_V0_xyz_ : packoffset(c0);
    float2 Test_V1_zw : packoffset(c1.z);
};


static float4 out_var_SV_Target;

struct SPIRV_Cross_Output
{
    float4 out_var_SV_Target : SV_Target0;
};

void frag_main()
{
    out_var_SV_Target = float4(Test_V0_xyz_, Test_V1_zw.x);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.out_var_SV_Target = out_var_SV_Target;
    return stage_output;
}
