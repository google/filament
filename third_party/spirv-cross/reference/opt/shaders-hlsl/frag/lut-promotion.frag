static const float _16[16] = { 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f };
static const float4 _60[4] = { 0.0f.xxxx, 1.0f.xxxx, 8.0f.xxxx, 5.0f.xxxx };
static const float4 _104[4] = { 20.0f.xxxx, 30.0f.xxxx, 50.0f.xxxx, 60.0f.xxxx };

static float FragColor;
static int index;

struct SPIRV_Cross_Input
{
    nointerpolation int index : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = _16[index];
    if (index < 10)
    {
        FragColor += _16[index ^ 1];
    }
    else
    {
        FragColor += _16[index & 1];
    }
    bool _63 = index > 30;
    if (_63)
    {
        FragColor += _60[index & 3].y;
    }
    else
    {
        FragColor += _60[index & 1].x;
    }
    float4 foobar[4] = _60;
    if (_63)
    {
        foobar[1].z = 20.0f;
    }
    int _91 = index & 3;
    FragColor += foobar[_91].z;
    FragColor += _104[_91].z;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    index = stage_input.index;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
