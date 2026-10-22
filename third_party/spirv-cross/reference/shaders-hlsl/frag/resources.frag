cbuffer CBuffer : register(b3)
{
    float4 cbuf_a : packoffset(c0);
};

cbuffer PushMe
{
    float4 registers_d : packoffset(c0);
};

Texture2D<float4> uSampledImage : register(t4);
SamplerState _uSampledImage_sampler : register(s4);
Texture2D<float4> uTexture : register(t5);
SamplerState uSampler : register(s6);

static float2 vTex;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float2 vTex : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    float4 c0 = uSampledImage.Sample(_uSampledImage_sampler, vTex);
    float4 c1 = uTexture.Sample(uSampler, vTex);
    float4 c2 = cbuf_a + registers_d;
    FragColor = (c0 + c1) + c2;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vTex = stage_input.vTex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
