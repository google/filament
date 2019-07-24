Texture2D<float4> uSampler : register(t0);
SamplerState _uSampler_sampler : register(s0);

static float4 FragColor;
static float2 vTexCoord;

struct SPIRV_Cross_Input
{
    float2 vTexCoord : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    float _19_tmp = uSampler.CalculateLevelOfDetail(_uSampler_sampler, vTexCoord);
    float2 _19 = _19_tmp.xx;
    FragColor = _19.xyxy;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vTexCoord = stage_input.vTexCoord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
