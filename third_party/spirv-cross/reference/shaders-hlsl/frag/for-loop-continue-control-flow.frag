static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = 0.0f.xxxx;
    int i = 0;
    int _36;
    for (;;)
    {
        if (i < 3)
        {
            int a = i;
            FragColor[a] += float(i);
            if (false)
            {
                _36 = 1;
            }
            else
            {
                int _41 = i;
                i = _41 + 1;
                _36 = _41;
            }
            continue;
        }
        else
        {
            break;
        }
    }
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
