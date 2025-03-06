cbuffer _6_7 : register(b0)
{
    column_major float2x4 _7_m0 : packoffset(c0);
    float4 _7_m1 : packoffset(c4);
};


static float2 _4;

struct SPIRV_Cross_Output
{
    float2 _4 : SV_Target0;
};

float2 _29()
{
    float2 _30 = mul(_7_m0, _7_m1);
    return _30;
}

void frag_main()
{
    _4 = _29();
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output._4 = _4;
    return stage_output;
}
