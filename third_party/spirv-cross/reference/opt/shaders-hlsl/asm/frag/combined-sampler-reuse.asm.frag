Texture2D<float4> uTex : register(t1);
SamplerState uSampler : register(s0);

static float4 FragColor;
static float2 vUV;

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = uTex.Sample(uSampler, vUV);
    FragColor += uTex.Sample(uSampler, vUV, int2(1, 1));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
