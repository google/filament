Texture2D<float4> uTex : register(t0);
SamplerState _uTex_sampler : register(s0);

static float4 FragColor;
static float4 vColor;
static float2 vTex;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
    float2 vTex : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = vColor * uTex.Sample(_uTex_sampler, vTex);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    vTex = stage_input.vTex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
