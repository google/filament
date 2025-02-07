struct UBO_1
{
    float4 v[64];
};

ConstantBuffer<UBO_1> ubos[] : register(b0, space2);
ByteAddressBuffer ssbos[] : register(t0, space3);
Texture2D<float4> uSamplers[] : register(t0, space0);
SamplerState uSamps[] : register(s0, space1);
Texture2D<float4> uCombinedSamplers[] : register(t4, space0);
SamplerState _uCombinedSamplers_sampler[] : register(s4, space0);

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
    int _23 = i + 10;
    int _34 = i + 40;
    FragColor = uSamplers[NonUniformResourceIndex(_23)].Sample(uSamps[NonUniformResourceIndex(_34)], vUV);
    int _50 = i + 10;
    FragColor = uCombinedSamplers[NonUniformResourceIndex(_50)].Sample(_uCombinedSamplers_sampler[NonUniformResourceIndex(_50)], vUV);
    int _66 = i + 20;
    int _70 = i + 40;
    FragColor += ubos[NonUniformResourceIndex(_66)].v[_70];
    int _84 = i + 50;
    int _88 = i + 60;
    FragColor += asfloat(ssbos[NonUniformResourceIndex(_84)].Load4(_88 * 16 + 0));
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
