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

static float4 _21;

void frag_main()
{
    float4 _24;
    _24 = _21;
    float4 _33;
    for (;;)
    {
        if (counter == 10)
        {
            _33 = 10.0f.xxxx;
            break;
        }
        else
        {
            _33 = 30.0f.xxxx;
            break;
        }
    }
    FragColor = _33;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    counter = stage_input.counter;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
