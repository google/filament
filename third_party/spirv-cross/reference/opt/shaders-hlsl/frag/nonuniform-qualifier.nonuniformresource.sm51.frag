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
    int _22 = vIndex + 10;
    int _32 = vIndex + 40;
    FragColor = uSamplers[NonUniformResourceIndex(_22)].Sample(uSamps[NonUniformResourceIndex(_32)], vUV);
    FragColor = uCombinedSamplers[NonUniformResourceIndex(_22)].Sample(_uCombinedSamplers_sampler[NonUniformResourceIndex(_22)], vUV);
    FragColor += ubos[NonUniformResourceIndex(vIndex + 20)].v[_32];
    FragColor += asfloat(ssbos[NonUniformResourceIndex(vIndex + 50)].Load4((vIndex + 60) * 16 + 0));
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
