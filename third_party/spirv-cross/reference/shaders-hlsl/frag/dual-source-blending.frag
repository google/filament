static float4 FragColor0;
static float4 FragColor1;

struct SPIRV_Cross_Output
{
    float4 FragColor0 : SV_Target0;
    float4 FragColor1 : SV_Target1;
};

void frag_main()
{
    FragColor0 = 1.0f.xxxx;
    FragColor1 = 2.0f.xxxx;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor0 = FragColor0;
    stage_output.FragColor1 = FragColor1;
    return stage_output;
}
