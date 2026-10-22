Texture2D<float4> uSampler[4] : register(t0);
SamplerState _uSampler_sampler[4] : register(s0);
Texture2D<float4> uTextures[4] : register(t8);
SamplerState uSamplers[4] : register(s4);

static int vIndex;
static float2 vTex;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    nointerpolation float2 vTex : TEXCOORD0;
    nointerpolation int vIndex : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

float4 sample_from_global()
{
    return uSampler[vIndex].Sample(_uSampler_sampler[vIndex], vTex + 0.100000001490116119384765625f.xx);
}

float4 sample_from_argument(Texture2D<float4> samplers[4], SamplerState _samplers_sampler[4])
{
    return samplers[vIndex].Sample(_samplers_sampler[vIndex], vTex + 0.20000000298023223876953125f.xx);
}

float4 sample_single_from_argument(Texture2D<float4> samp, SamplerState _samp_sampler)
{
    return samp.Sample(_samp_sampler, vTex + 0.300000011920928955078125f.xx);
}

void frag_main()
{
    FragColor = 0.0f.xxxx;
    FragColor += uTextures[2].Sample(uSamplers[1], vTex);
    FragColor += uSampler[vIndex].Sample(_uSampler_sampler[vIndex], vTex);
    FragColor += sample_from_global();
    FragColor += sample_from_argument(uSampler, _uSampler_sampler);
    FragColor += sample_single_from_argument(uSampler[3], _uSampler_sampler[3]);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    vTex = stage_input.vTex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
