Texture2D<float4> uCombined[4] : register(t0);
SamplerState _uCombined_sampler[4] : register(s0);
Texture2D<float4> uTex[4] : register(t4);
SamplerState uSampler[4] : register(s8);
RWTexture2D<float4> uImage[8] : register(u12);

static float4 gl_FragCoord;
static float2 vTex;
static int vIndex;

struct SPIRV_Cross_Input
{
    float2 vTex : TEXCOORD0;
    nointerpolation int vIndex : TEXCOORD1;
    float4 gl_FragCoord : SV_Position;
};

float4 sample_in_function(Texture2D<float4> samp, SamplerState _samp_sampler)
{
    return samp.Sample(_samp_sampler, vTex);
}

float4 sample_in_function2(Texture2D<float4> tex, SamplerState samp)
{
    return tex.Sample(samp, vTex);
}

void frag_main()
{
    float4 color = uCombined[vIndex].Sample(_uCombined_sampler[vIndex], vTex);
    color += uTex[vIndex].Sample(uSampler[vIndex], vTex);
    int _72 = vIndex + 1;
    color += sample_in_function(uCombined[_72], _uCombined_sampler[_72]);
    color += sample_in_function2(uTex[vIndex + 1], uSampler[vIndex + 1]);
    uImage[vIndex][int2(gl_FragCoord.xy)] = color;
}

void main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    vTex = stage_input.vTex;
    vIndex = stage_input.vIndex;
    frag_main();
}
