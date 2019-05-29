cbuffer _4_5 : register(b0)
{
    column_major float2x4 _5_m0 : packoffset(c0);
    float4 _5_m1 : packoffset(c4);
};


static float2 _3;

struct SPIRV_Cross_Output
{
    float2 _3 : SV_Target0;
};

float2 _23()
{
    float2 _25 = mul(_5_m0, _5_m1);
    return _25;
}

void frag_main()
{
    _3 = _23();
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output._3 = _3;
    return stage_output;
}
