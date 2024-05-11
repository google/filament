static float2 value;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float2 value : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    bool2 _25 = bool2(value.x == 0.0f, value.y == 0.0f);
    bool2 bools1 = bool2(!_25.x, !_25.y);
    bool2 bools2 = bool2(value.x <= float2(1.5f, 0.5f).x, value.y <= float2(1.5f, 0.5f).y);
    FragColor = float4(1.0f, 0.0f, float(bools1.x), float(bools2.x));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    value = stage_input.value;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
