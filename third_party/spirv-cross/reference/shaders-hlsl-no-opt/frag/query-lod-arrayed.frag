Texture2DArray<float4> uTex : register(t0);
SamplerState uSamp : register(s1);

static float2 FragCoord;
static float2 vUV;

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float2 FragCoord : SV_Target0;
};

void frag_main()
{
    float _23_tmp = uTex.CalculateLevelOfDetail(uSamp, vUV);
    float2 _23 = _23_tmp.xx;
    FragCoord = _23;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragCoord = FragCoord;
    return stage_output;
}
