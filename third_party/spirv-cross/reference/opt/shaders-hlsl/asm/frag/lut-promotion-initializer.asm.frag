static const float _46[16] = { 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 2.0f, 3.0f, 4.0f };
static const float4 _76[4] = { 0.0f.xxxx, 1.0f.xxxx, 8.0f.xxxx, 5.0f.xxxx };
static const float4 _90[4] = { 20.0f.xxxx, 30.0f.xxxx, 50.0f.xxxx, 60.0f.xxxx };

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
    float4 foobar[4] = _76;
    float4 baz[4] = _76;
    FragColor = _46[index];
    if (index < 10)
    {
        FragColor += _46[index ^ 1];
    }
    else
    {
        FragColor += _46[index & 1];
    }
    bool _99 = index > 30;
    if (_99)
    {
        FragColor += _76[index & 3].y;
    }
    else
    {
        FragColor += _76[index & 1].x;
    }
    if (_99)
    {
        foobar[1].z = 20.0f;
    }
    int _37 = index & 3;
    FragColor += foobar[_37].z;
    baz = _90;
    FragColor += baz[_37].z;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    index = stage_input.index;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
