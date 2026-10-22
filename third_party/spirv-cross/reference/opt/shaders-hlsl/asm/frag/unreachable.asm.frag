static int counter;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    nointerpolation int counter : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    float4 _46;
    for (;;)
    {
        if (counter == 10)
        {
            _46 = 10.0f.xxxx;
            break;
        }
        else
        {
            _46 = 30.0f.xxxx;
            break;
        }
    }
    FragColor = _46;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    counter = stage_input.counter;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
