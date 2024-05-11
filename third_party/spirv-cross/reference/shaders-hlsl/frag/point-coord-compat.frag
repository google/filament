static float2 FragColor;

struct SPIRV_Cross_Output
{
    float2 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = float2(0.5f, 0.5f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
