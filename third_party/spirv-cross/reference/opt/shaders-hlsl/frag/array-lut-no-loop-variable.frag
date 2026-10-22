static const float _17[5] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    for (int _46 = 0; _46 < 4; )
    {
        int _33 = _46 + 1;
        FragColor += _17[_33].xxxx;
        _46 = _33;
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
