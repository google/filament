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
    bool _29;
    for (;;)
    {
        _29 = counter == 10;
        if (_29)
        {
            break;
        }
        else
        {
            break;
        }
    }
    bool4 _35 = _29.xxxx;
    FragColor = float4(_35.x ? 10.0f.xxxx.x : 30.0f.xxxx.x, _35.y ? 10.0f.xxxx.y : 30.0f.xxxx.y, _35.z ? 10.0f.xxxx.z : 30.0f.xxxx.z, _35.w ? 10.0f.xxxx.w : 30.0f.xxxx.w);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    counter = stage_input.counter;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
