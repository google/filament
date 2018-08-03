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

void frag_main()
{
    int _72 = vIndex + 1;
    uImage[vIndex][int2(gl_FragCoord.xy)] = ((uCombined[vIndex].Sample(_uCombined_sampler[vIndex], vTex) + uTex[vIndex].Sample(uSampler[vIndex], vTex)) + uCombined[_72].Sample(_uCombined_sampler[_72], vTex)) + uTex[_72].Sample(uSampler[_72], vTex);
}

void main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    vTex = stage_input.vTex;
    vIndex = stage_input.vIndex;
    frag_main();
}
