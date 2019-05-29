static float3 FragColor;

struct SPIRV_Cross_Output
{
    float3 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = float3(asfloat(0x7f800000u), asfloat(0xff800000u), asfloat(0x7fc00000u));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
