static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = 0.0f.xxxx;
    for (int _43 = 0; _43 < 3; )
    {
        FragColor[_43] += float(_43);
        _43++;
        continue;
    }
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
