struct UBO_1_1
{
    float4 v[64];
};

ConstantBuffer<UBO_1_1> ubos[] : register(b0, space3);
ByteAddressBuffer ssbos[] : register(t0, space4);
Texture2D<float4> uSamplers[] : register(t0, space0);
SamplerState uSamps[] : register(s0, space2);
Texture2D<float4> uCombinedSamplers[] : register(t0, space1);
SamplerState _uCombinedSamplers_sampler[] : register(s0, space1);

static int vIndex;
static float4 FragColor;
static float2 vUV;

struct SPIRV_Cross_Input
{
    nointerpolation int vIndex : TEXCOORD0;
    float2 vUV : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    int i = vIndex;
    FragColor = uSamplers[NonUniformResourceIndex(i + 10)].Sample(uSamps[NonUniformResourceIndex(i + 40)], vUV);
    int _47 = i + 10;
    FragColor = uCombinedSamplers[NonUniformResourceIndex(_47)].Sample(_uCombinedSamplers_sampler[NonUniformResourceIndex(_47)], vUV);
    FragColor += ubos[NonUniformResourceIndex(i + 20)].v[i + 40];
    FragColor += asfloat(ssbos[NonUniformResourceIndex(i + 50)].Load4((i + 60) * 16 + 0));
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vIndex = stage_input.vIndex;
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
